//-*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
#include "commandhandler.hh"
#include <dirent.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>
#include "error.hh"

class VersionHandler: public CommandHandler {
public:
	std::string name() const {return "version";}
	void handle(PackageSocket & s) {s.write("1.0");}
};
CommandHandler * produceVersionHandler() {return new VersionHandler();}

class PingHandler: public CommandHandler {
public:
	std::string name() const {return "ping";}
	void handle(PackageSocket & s) {s.write("pong");}
};
CommandHandler * producePingHandler() {return new PingHandler();}

class ListHandler: public CommandHandler {
public:
	std::string name() const {return "list";}
	void handle(PackageSocket & s) {
		DIR * d = opendir("entries");
		if(d != NULL) {
			while(struct dirent * e = readdir(d)) {
				if(e->d_name[0] == '.' || e->d_name[0] == '\0') continue;
				s.write(e->d_name);
			}
			closedir(d);
		}
		s.write("");
	}
};
CommandHandler * produceListHandler() {return new ListHandler();}

class DestroyHandler: public CommandHandler {
public:
	std::string name() const {return "destroy";}
	void handle(PackageSocket & s) {
		char buff[1024];
		size_t x=1024;
		while(!s.read(buff,x));
		bool clean=true;
		for(size_t i=0; i < x; ++i) 
			if(buff[i] == '.' || buff[i] == '/' || buff[i] == '\'' 
			   || buff[i] == '$' || buff[i] == '#') clean=false;
		if(!clean) {
			s.write("invalid name");
			return;
		}
		buff[x] = '\0';
		char buff2[1124];
		sprintf(buff2,"rm -rf 'entries/%s'",buff);
		system(buff2);
		s.write("success");
	};
};
CommandHandler * produceDestroyHandler() {return new DestroyHandler();}
