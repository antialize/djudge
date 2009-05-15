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

class ImportHandler: public CommandHandler {
	std::string name() const {return "import";}
	void handle(PackageSocket & s) {
		int pipefd[2];
		if(pipe(pipefd) == -1) THROW_PE("pipe() failed\n");
		pid_t r = fork();
		if(r == 0) {
			close(pipefd[1]);
			if(dup2(pipefd[0],0) == -1) {perror("dup2() failed"); exit(-1);}
			execlp("tar","tar","-xjvC","entries", NULL);
			//execlp("hexdump","hexdump","-C", NULL);
			perror("execlp() falied"); 
			exit(-1);
		} else {
			if(r == -1) THROW_PE("fork() failed\n");
			size_t l = 10240;
			char buff[l];
			close(pipefd[0]);
			while(!s.read(buff,l)) {
				if(write(pipefd[1], buff, l) != l) 
					THROW_PE("write failed");
			}
			if(write(pipefd[1], buff, l) != l) 
					THROW_PE("write failed");
			close(pipefd[1]);
			int status;
			if(waitpid(r, &status, 0) == -1) THROW_PE("waitpid() failed\n");
			if(WIFEXITED(status) && WEXITSTATUS(status) == 0) 
				s.write("success");
			else 
				s.write("failed");
		}
	}
};
CommandHandler * produceImportHandler() {return new ImportHandler();}
