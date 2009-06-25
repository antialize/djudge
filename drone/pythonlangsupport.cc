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
#include "fs.hh"
#include "globals.hh"
#include "langsupport.hh"
#include "results.hh"
#include "saferun.hh"
#include <iostream>
using namespace std;

class PythonLangSupport: public LangSupport {
public:
	std::string sourceName(std::string base) {return base+".py";}
	int removeSource(std::string base) {return 0;}
	int removeBinary(std::string base) {return unlink((base+".py").c_str());}

	bool hasSource(std::string name) {
		string path=name+".py";
		cout << path << endl;
		return access(path.c_str(),R_OK) == 0;
	}
	bool compile(std::string, int user, int group, PackageSocket & s) {return true;}
	void restrictRun(std::string name ,bool entryAccess) {
		string path = absolutePath(name)+".py";
		string s;
		string py=absolutePath(proxyPath+"/proxy_python");
		if(entryAccess) s = "  " + getCWD() + "/**   rw,\n";
		appArmorLoadProfile(
			"#include <tunables/global>\n"
			"%s {\n"
			"  /usr/bin/python*            rix,\n"
			"  #include <abstractions/base>\n"
			"  #include <abstractions/python>\n"
			"  %s                          r,\n"
			"  %s"
			"}\n",
			py.c_str(),
			path.c_str(),
			s.c_str());
	};
	
	void unrestrictRun(std::string name) {
		string py=absolutePath(proxyPath+"/proxy_python");
		appArmorRemoveProfile(py.c_str());		
	};

	int run(std::string name, 
			int in, int out, int err, 
			size_t memoryLimit,
			size_t outputLimit,
			float & time,
			int user,
		    int group) {
		string path=name+".py";
		string py=proxyPath+"/proxy_python";
		return saferun(in,out,err,memoryLimit,outputLimit,user,group,time,0,py.c_str(),py.c_str(),path.c_str(),NULL);
	}
	float rank() {return 99;}
	string name() {return "python";}
};

LangSupport * producePythonLangSupport() {return new PythonLangSupport();}

