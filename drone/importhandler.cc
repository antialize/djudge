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
#include "commandhandler.hh"
#include "config.hh"
#include "error.hh"
#include "fs.hh"
#include "globals.hh"
#include "langsupport.hh"
#include "results.hh"
#include <boost/program_options.hpp>
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
//Opun import the following must be done
//0. Extract the archive
//1. Possible generate (additional) inputs
//2. Possible Compile a Special judge
//3. Compile all sample solutions and time them on all inputs
using namespace std;
class ImportHandler: public CommandHandler {
public:
	std::string name() const {return "import";}
	void handle(PackageSocket & s) {
		std::string p;
		try {
			umask(~S_IRWXU);
			//Read the name of the new entity
			std::string name = s.readString(1024);
			printf("==============> importing %s <==============\n",name.c_str());

			p = entriesPath + "/" + name;
			pid_t r = fork();
			if(r == 0) {
				setregid(droneGroup, droneGroup);
				setreuid(droneUser, droneUser);
				mkdir(entriesPath.c_str(),0700);
				if(chdir(entriesPath.c_str()) == -1) exit(1);
				if(mkdir(name.c_str(),0700) == -1 && errno != EEXIST) exit(1);
				if(chdir(name.c_str()) == -1) exit(1);
				mkdir("inputs",0700);
				mkdir("inputs",0700);
				mkdir("outputs",0700);
				mkdir("times",0700);
				exit(0);
			} 
			int status;
			if(r == -1) THROW_PE("fork() failed\n");
			if(waitpid(r, &status, 0) == -1) THROW_PE("waitpid() failed\n");
			if(!WIFEXITED(status) || WEXITSTATUS(status) != 0) THROW_E("Failed creating directory structure\n");
			if(chdir(p.c_str()) == -1) THROW_PE("chdir() failed\n");

			//Extract the archive on the fly
			int pipefd[2];
			if(pipe(pipefd) == -1) THROW_PE("pipe() failed\n");
			r = fork();
			if(r == 0) {
				close(pipefd[1]);
				if(dup2(pipefd[0],0) == -1) {perror("dup2() failed"); exit(-1);}
				setregid(droneGroup, droneGroup);
				setreuid(droneUser,droneUser);
				execlp("tar","tar","-xjvC",".", NULL);
				perror("execlp() falied"); 
				exit(-1);
			}
			if(r == -1) THROW_PE("fork() failed\n");
			close(pipefd[0]);
			s.readFD(pipefd[1]);
			close(pipefd[1]);
			if(waitpid(r, &status, 0) == -1) THROW_PE("waitpid() failed\n");
			if(!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
				rmrf(p.c_str());
				s.write(XSTR(RUN_EXTRACT_ERROR));
				std::ostringstream e;
				e << "unpacking faild with exitcode " << WEXITSTATUS(status);
				s.write(e.str());
				return;
			}

			printf("Checking the config.ini file\n");
			try {
				Config * c = parseConfig("config.ini");
				delete c;
			} catch(std::exception & e) { 
				rmrf(p.c_str());
				s.write(XSTR(RUN_EXTRACT_ERROR));
 				std::ostringstream _;
 				_ << "Error parsing config.ini: " << e.what();
 				s.write(_.str());
				return;
			}

			printf("Generating additional inputs\n");
			//Generate additional inputs here
			for(langByRank_t::iterator i = langByRank.begin(); 
				i != langByRank.end(); ++i)  {
				LangSupport * l = i->second;
				if(!l->hasSource("inputgenerator")) continue;
				if(!l->compile("inputgenerator", droneUser, droneGroup, s)) return;
				float time=maxTime;
				l->restrictRun("inputgenerator", true);
				int r = l->run("inputgenerator", 0, 1, 2, maxMemory, maxOutput, time, droneUser, droneGroup);
				if(r != RUN_SUCCESS) {
					rmrf(p.c_str());
					char buff[1024];
					sprintf(buff,"%d",r);
					s.write(buff);
					sprintf(buff,"%s inputgenerator failed",l->name().c_str());
					s.write(buff);
					return;
				}
				l->unrestrictRun("inputgenerator");
			}
		
			printf("Generating outputs and times\n");
			//Generate expected outputs
			for(langByRank_t::iterator i = langByRank.begin(); 
				i != langByRank.end(); ++i)  {
				LangSupport * l = i->second;
				if(!l->hasSource("reference")) continue;
				if(!l->compile("reference", droneUser, droneGroup, s)) return;
				l->restrictRun("reference", false);
				DIR * d = opendir("inputs");
				if(d == NULL) THROW_PE("opendir() failed");
				while(struct dirent * e = readdir(d)) {
					if(e->d_name[0] == '.' || e->d_name[0] == '\0') continue;
					char buff[1024];
					sprintf(buff,"inputs/%s",e->d_name);
					float time=maxTime;
					int in=open(buff,O_RDONLY);
					sprintf(buff,"outputs/%s",e->d_name);
					int out=open(buff,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
					int r = l->run("reference",in,out,2,maxMemory, maxOutput,time, droneUser, droneGroup);
					printf("Running %s reference solution on %s, finished in time %f with result %d\n",
						   l->name().c_str(), e->d_name, time, r);
					close(in);
					close(out);
					chown(buff, droneUser, droneGroup);
					if(r != RUN_SUCCESS) {
						rmrf(".");
						char buff[1024];
						sprintf(buff,"%d",r);
						s.write(buff);
						snprintf(buff,1023,"%s reference soluction falied on input %s",l->name().c_str(), e->d_name);
						buff[1023] = '\0';
						s.write(buff);
						return;
					}
					sprintf(buff,"times/%s.%s",e->d_name,l->name().c_str());
					FILE * f = fopen(buff,"w");
					if(f == NULL) THROW_PE("fopen() failed");
					fprintf(f,"%f\n",time);
					fclose(f);
					if(chown(buff, droneUser, droneGroup) == -1) THROW_PE("chown() failed");
				}
				l->unrestrictRun("reference");
				closedir(d);
			}
			s.write(XSTR(RUN_SUCCESS));
			s.write(name + " imported sucessfully");
		} catch(CommonException & e) {
			if(p != "") rmrf(p.c_str());
			s.write(XSTR(RUN_INTERNAL_ERROR));
			s.write(e.what());
		}
	}
};
CommandHandler * produceImportHandler() {return new ImportHandler();}



