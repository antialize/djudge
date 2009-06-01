//-*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
/*
 * djudge, an online judge solution.
 * Copyright (C) 2009 Jakob Truelsen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "apparmor.hh"
#include "error.hh"
#include "fs.hh"
#include "globals.hh"
#include "langsupport.hh"
#include "results.hh"
#include "saferun.hh"
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <sys/wait.h>
using namespace std;

class CCLangSupport: public LangSupport {
public:
	std::string sourceName(std::string base) {return base+".cc";}
	int removeSource(std::string base) {return unlink((base+".cc").c_str());}
	int removeBinary(std::string base) {return unlink((base+".out").c_str());}

	bool hasSource(std::string name) {
		string path=name+".cc";
		return access(path.c_str(),R_OK) == 0;
	}
	float rank() {return 1;}
	string name() {return "cc";}
	
	void restrictRun(std::string name ,bool entryAccess) {
		string path = absolutePath(name);
		string s;
		if(entryAccess) s = "  " + getCWD() + "/**   rw,\n";
		appArmorLoadProfile(
			"#include <tunables/global>\n"
			"%s.out {\n"
			"  #include <abstractions/base>\n"
			"  %s.out                 r,\n"
			"  %s"
			"}\n",
			path.c_str(),
			path.c_str(),
			s.c_str());
	}
			
	void unrestrictRun(std::string name) {
		appArmorRemoveProfile(absolutePath(name+".out").c_str());
	}

	bool compile(std::string name, int user, int group, PackageSocket & s) {
		string src=absolutePath(name+".cc");
		string dst=absolutePath(name+".out");
		float time=20;
		bool result=false;
		std::string gpp=proxyPath+"/proxy_g++";
		appArmorLoadProfile(
			"#include <tunables/global>\n"
			"%s {\n"
			"  #include <abstractions/base>\n"
			"  %s                          r,\n"
			"  /tmp/                       rw,\n"
			"  /tmp/**                     rw,\n"
			"  %s                          r,\n"
			"  %s                          rw,\n"
			"  /usr/lib/gcc/*/*/cc1plus    rix,\n"
			"  /usr/lib/gcc/*/*/collect2   rix,\n"
			"  /usr/bin/as                 rix,\n"
			"  /usr/include/**             r,\n"
			"  /usr/bin/ld                 rix,\n"
			"  /usr/bin/nm                 rix,\n"
			"  /usr/bin/strip              rix,\n"
			"  /usr/bin/g++*               rix,\n"
			"}\n",
			gpp.c_str(),gpp.c_str(),
			src.c_str(),
			dst.c_str());
		int p[2];
		if(pipe(p) == -1) THROW_PE("pipe() failed");
		pid_t f = fork();
		if(f == 0) {
			close(p[0]);
			exit(saferun(0,p[1],p[1],100*1024*1024,0,user,group,time,10,"g++","g++","-O2" , "-Wall" , "-W", "-o", dst.c_str(), src.c_str(), NULL));
		}
		if(f == -1) THROW_PE("fork() failed");
		close(p[1]);
		std::string res;
		while(true) {
			char buff[1024*128];
			int r = read(p[0],buff,1024*128);
			if(r == -1) THROW_PE("read() failed");
			if(r == 0) break;
			res += buff;
		}
		close(p[0]);
		int stat;
		if(waitpid(f,  &stat, 0) == -1) THROW_PE("waitpid() failed");
		if(!WIFEXITED(stat)) {
			s.write(XSTR(RUN_INTERNAL_ERROR));
			s.write("Somehow I was terminated\n");
		} else {
			switch(WEXITSTATUS(stat)) {
			case RUN_SUCCESS:
				result = true;
				break;
			case RUN_EXIT_NOT_ZERO:
				s.write(XSTR(RUN_COMPILATION_ERROR));
				s.write(res.c_str());
				break;
			case RUN_TIME_LIMIT_EXCEEDED:
				s.write(XSTR(RUN_COMPILATION_TIME_LIMIT_EXCEEDED));
				s.write(res.c_str());
				break;
			default:
				s.write(XSTR(RUN_INTERNAL_ERROR));
				s.write(res.c_str());
				break;
			}
		}
		appArmorRemoveProfile(gpp.c_str());
		return result;
	}

	int run(std::string name, 
			int in, int out, int err, 
			size_t memoryLimit,
			size_t outputLimit,
			float & time,
			int user,
			int group
		) {
		string path;
		if(name[0] == '/') path=name+".out";
		else path="./"+name+".out";
		return saferun(in,out,err,memoryLimit,outputLimit,user,group,time,0,path.c_str(),path.c_str(),NULL);;
	}
};

LangSupport * produceCCLangSupport() {return new CCLangSupport();}
