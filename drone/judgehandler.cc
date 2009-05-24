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
#include "error.hh"
#include "globals.hh"
#include "langsupport.hh"
#include "results.hh"
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


int judgemain(int out) {
	char a[10240];
	char b[10240];
	size_t au=0;
	size_t bu=0;
	size_t al=0;
	size_t bl=0;
	bool eoa=false;
	bool eob=false;
	bool precentationError=false;
	while(!eoa || ! eob) {
		if(!eoa && au == al) {
			int r = read(0,a,10240);
			au = 0;
			if(r <= 0) eoa=true;
			else al = r;
		}
		if(!eob && bu == bl) {
			int r = read(out,b,10240);
			bu = 0;
			if(r <= 0) eob=true;
			else bl = r;
			a[al] = '\0';
			b[bl] = '\0';
		}
		int na=eoa?-1:a[au];
		int nb=eob?-1:b[bu];
		if(na == nb) {
			au++;
			bu++;
		} else 	if(na == '\n' || na == ' ' || na == '\r' || na == '\t') {
			au++;
			precentationError=true;
		} else 	if(nb == '\n' || nb == ' ' || nb == '\r' || nb == '\t') {
			bu++;
			precentationError=true;
		} else {
			exit(1);
		}
	}
	exit(precentationError?2:0);
}

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
			s.readFD(fd);
			close(fd);
			cout << "===========> Judging " << entry << " <==============" << endl;
			bool r = l->compile(base, nobodyUser ,nobodyGroup,s);
			l->removeSource(base);
			if(r == false) return;
			string dir=entriesPath + "/" + entry;
			if(chdir(dir.c_str()) == -1) THROW_PE("chdir() failed");
 			DIR * d = opendir("inputs");
			if(d == NULL)  {
				s.write(XSTR(RUN_INTERNAL_ERROR));
				s.write("Unable to open input");
				l->removeBinary(base);
				return;
			}
			string msg="success";
			int res=RUN_SUCCESS;
			l->restrictRun(base,false);
			while(struct dirent * e = readdir(d)) {
				if(e->d_name[0] == '.' || e->d_name[0] == '\0') continue;
			 	char input[1024];
 				sprintf(input,"inputs/%s",e->d_name);
 				char output[1024];
 				sprintf(output,"outputs/%s",e->d_name);
 				int in1=open(input,O_RDONLY);
 				if(in1 == -1) THROW_PE("open() failed\n");
 				int in2=open(input,O_RDONLY);
 				if(in2 == -1) THROW_PE("open() failed\n");
 				int out=open(output,O_RDONLY);
				int p[2];
 				if(pipe(p) == -1) THROW_PE("pipe() failed");
 				int judge = fork();
				if(judge == 0) {
 					close(in1);
 					close(p[1]);
					dup2(p[0],0);
					judgemain(out);
					exit(3);
				}
 				if(judge == -1) THROW_PE("fork() failed");
				float time=10;
				int r = l->run(base, in1, p[1], 2 , 1024*1024*128, 0, time, nobodyUser, nobodyGroup);
				close(in1);
				close(p[1]);
				int s;
				waitpid(judge,&s,0);
				if(r != 0) {
					res = r;
					msg = "Error on runtime";
					break;
				} else if(!WIFEXITED(s) || WEXITSTATUS(s) < 0 || WEXITSTATUS(s) > 2) {
					res = RUN_INTERNAL_ERROR;
					msg = "The judge program failed";
					break;
				} else if(WEXITSTATUS(s) == 1) {
					res = RUN_WRONG_OUTPUT;
					msg = "Your program did not output the correct output";
					break;
				} else if(WEXITSTATUS(s) == 2) {
					res = RUN_PRESENTATION_ERROR;
					msg = "Your output was almost correct, but the whitespace was wrong";
					break;
				}
			}
			l->unrestrictRun(base);
			l->removeBinary(base);
			char buff[10];
			sprintf(buff,"%d",res);
			s.write(buff);
			s.write(msg.c_str());
		} catch(CommonException & e) {
			s.write(XSTR(RUN_INTERNAL_ERROR));
			s.write(e.what());
		}
	}
};
CommandHandler * produceJudgeHandler() {return new JudgeHandler();}  
