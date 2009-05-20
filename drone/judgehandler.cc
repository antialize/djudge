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

using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "error.hh"
#include "langsupport.hh"
#include "globals.hh"
#include "results.hh"
#include <sys/types.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

class JudgeHandler: public CommandHandler {
public:
	std::string name() const {return "judge";}
	void handle(PackageSocket & s) {
		try {
			umask(~S_IRWXU);
			string entry = s.readString(1024);
			string language = s.readString(1024);
			string verbosity = s.readString(1024);
			if(langByName.count(language) == 0) THROW_E("Invalid language");
			LangSupport * l = langByName[language];
			string base = tempnam("/tmp","djudge");
			int fd = open(l->sourceName(base).c_str(), O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
			if(fd == -1) THROW_PE("open() failed");
			if(fchown(fd, nobodyUser, nobodyGroup) == -1) THROW_PE("fchown() failed");
			size_t n=1024*10;
			char buff[n];
			while(!s.read(buff,n)) 
				if(write(fd,buff,n) != (int)n) 
					THROW_PE("write() failed");
			close(fd);
			bool r = l->compile(base, nobodyUser ,nobodyGroup,s);
			l->removeSource(base);
			if(r == false) return;
			string dir=entriesPath + "/" + entry;
			if(chdir(dir.c_str()) == -1) THROW_PE("chdir() failed");
 			DIR * d = opendir("inputs");
			if(d == NULL)  {
				s.write(XSTR(RUN_INVALID_ENTRY));
				s.write("");
				l->removeBinary(base);
				return;
			}
			while(struct dirent * e = readdir(d)) {
				if(e->d_name[0] == '.' || e->d_name[0] == '\0') continue;
				char input[1024];
				sprintf(input,"inputs/%s",e->d_name);
				char output[1024];
				sprintf(input,"outputs/%s",e->d_name);
				int in1=open(input,O_RDONLY);
				if(in1 == -1) THROW_PE("open() failed\n");
				int in2=open(input,O_RDONLY);
				if(in2 == -1) THROW_PE("open() failed\n");
				int out=open(output,O_RDONLY);
				//There might be no output if we are using some special judge
				
				int p[2];
				if(pipe(p) == -1) THROW_PE("pipe() failed");
				int judge = fork();
				if(judge == 0) {
					close(in1);
					close(p[1]);
					dup2(p[0],0);
					if(out == 3 && in2 == 4) {
						dup2(in2,5);
						close(in2);
						dup2(out,4);
						close(out);
						dup2(5,3);
						close(5);
					} else if(out == 3) {
						dup2(out,4);
						close(out);
						dup2(in2,3);
						close(in2);
					} else {
						dup2(in2,3);
						close(in2);
						dup2(out,4);
						close(out);
					}
					//Run judge here
				}
				if(judge == -1) THROW_PE("fork() failed");
				
				int exe = fork();
				if(exe == 0) {
					close(in2);
					close(out);
					close(p[0]);
					//run(in1,p[1],2);
				}
				//Create the judging chain
			}
			//Open entity
			//Run instances
			//Calculate correct answer
			s.write(XSTR(RUN_SUCCESS));
		} catch(CommonException & e) {
			s.write(XSTR(RUN_INTERNAL_ERROR));
			s.write(e.what());
		}
	}
};
CommandHandler * produceJudgeHandler() {return new JudgeHandler();}  
