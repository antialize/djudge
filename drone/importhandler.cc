//-*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
#include "commandhandler.hh"
#include <dirent.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>
#include "error.hh"
#include "langsupport.hh"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "results.hh"
#include "globals.hh"

//Opun import the following must be done
//0. Extract the archive
//1. Possible generate (additional) inputs
//2. Possible Compile a Special judge
//3. Compile all sample solutions and time them on all inputs

class ImportHandler: public CommandHandler {
public:
	std::string name() const {return "import";}
	void handle(PackageSocket & s) {
		umask(~S_IRWXU);
		//Read the name of the new entity
		char name[1024];
		size_t x=1023;
		while(!s.read(name,x)) {}
		name[x] = '\0';
		printf("==============> importing %s <==============\n",name);
		//Extract the archive on the fly
		int pipefd[2];
		if(pipe(pipefd) == -1) THROW_PE("pipe() failed\n");
		pid_t r = fork();
		if(r == 0) {
			close(pipefd[1]);
			if(dup2(pipefd[0],0) == -1) {perror("dup2() failed"); exit(-1);}
			setregid(droneGroup, droneGroup);
			setreuid(droneUser,droneUser);
			execlp("tar","tar","-xjvC","entries", NULL);
			perror("execlp() falied"); 
			exit(-1);
		}
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
		if(!WIFEXITED(status) || WEXITSTATUS(status) == 0) s.write("failed");

		if(chdir(entriesPath.c_str()) == -1) THROW_PE("chdir() failed\n");
		if(chdir(name) == -1) THROW_PE("chdir() failed\n");
		printf("Generating additional inputs\n");
		
        //Generate additional inputs here
		r = fork();
		if(r == 0) {
			setregid(droneGroup, droneGroup);
			setreuid(droneUser, droneUser);
			mkdir("inputs",0700);
			mkdir("outputs",0700);
			mkdir("times",0700);
			exit(0);
		} 
		if(r == -1) THROW_PE("fork() failed\n");
		if(waitpid(r, &status, 0) == -1) THROW_PE("waitpid() failed\n");
		if(!WIFEXITED(status) || WEXITSTATUS(status) == 0) s.write("failed");

		for(langByRank_t::iterator i = langByRank.begin(); 
			i != langByRank.end(); ++i)  {
			LangSupport * l = i->second;
			if(!l->hasSource("inputGenerator")) continue;
			if(l->compile("inputGenerator", droneUser, droneGroup) != RUN_SUCCESS) goto fail;
			float time=60;
			l->restrictRun("inputGenerator", true);
			if(l->run("inputGenerator", 0, 1, 2, 100 * 1024 * 1024, 0, time, droneUser, droneGroup) != RUN_SUCCESS) goto fail;
			l->unrestrictRun("inputGenerator");
		}
		
		printf("Generating outputs and times\n");
		//Generate expected outputs
		for(langByRank_t::iterator i = langByRank.begin(); 
			i != langByRank.end(); ++i)  {
			LangSupport * l = i->second;
			if(!l->hasSource("reference")) continue;
			if(l->compile("reference", droneUser, droneGroup) != RUN_SUCCESS) {
				printf("Compile failed\n");
				goto fail;
			}
			l->restrictRun("inputGenerator", false);
			DIR * d = opendir("inputs");
			if(d == NULL) goto fail;
			while(struct dirent * e = readdir(d)) {
				if(e->d_name[0] == '.' || e->d_name[0] == '\0') continue;
				char buff[1024];
				sprintf(buff,"inputs/%s",e->d_name);
				float time=60;
				int in=open(buff,O_RDONLY);
				sprintf(buff,"outputs/%s",e->d_name);
				int out=open(buff,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
				int r = l->run("reference",in,out,2,1024*1024*100,1024*1024*100,time, droneUser, droneGroup);
				printf("Running %s reference solution on %s, finished in time %f with result %d\n",
					   l->name().c_str(), e->d_name, time, r);
				close(in);
				close(out);
				chown(buff, droneUser, droneGroup);
				if(r != RUN_SUCCESS) goto fail;
				sprintf(buff,"times/%s.%s",e->d_name,l->name().c_str());
				FILE * f = fopen(buff,"w");
				fprintf(f,"%f\n",time);
				fclose(f);
				chown(buff, droneUser, droneGroup);
			}
			l->unrestrictRun("inputGenerator");
			closedir(d);
		}
		s.write("success");
		chdir("../..");
		return;
	fail:
		s.write("failed");
		chdir("../..");
	}
};
CommandHandler * produceImportHandler() {return new ImportHandler();}



