//-*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
#include "apparmor.hh"
#include <stdarg.h> 
#include "error.hh"
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdarg.h>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

void appArmorLoadProfile(const char * profile, ...) {
	char buff[1024*128];
	va_list ap;
	va_start(ap, profile);
	vsnprintf(buff, sizeof(buff)-1,profile, ap);
	va_end(ap);
	buff[sizeof(buff)-1] = '\0';
	
	int p[2];
	pipe(p);
	pid_t f = fork();
	if(f==0) {
		close(p[1]);
		dup2(p[0],0);
		execlp("apparmor_parser","apparmor_parser","-r",NULL);
		exit(1);
	}
	//printf("%s\n",buff);
	close(p[0]);
	if(write(p[1], buff, strlen(buff)) != (int)strlen(buff)) THROW_PE("waitpid");
	close(p[1]);
	int status;
	if(waitpid(f, &status, 0) == -1) THROW_PE("waitpid");
	//printf("a\n");
	if(!WIFEXITED(status) || WEXITSTATUS(status) != 0) THROW_E("Loading apparmor profile failed");
	//printf("b\n");
}


void appArmorRemoveProfile(const char * path) {
	char buff[10240];
	sprintf(buff,"%s {\n}\n",path);
	int p[2];
	pipe(p);
	pid_t f = fork();
	if(f==0) {
		close(p[1]);
		dup2(p[0],0);
		execlp("apparmor_parser","apparmor_parser","-R",NULL);
		exit(1);
	}
	close(p[0]);
	if(write(p[1], buff, strlen(buff)) != (int)strlen(buff)) THROW_PE("waitpid");
	close(p[1]);
	int status;
	if(waitpid(f, &status, 0) == -1) THROW_PE("waitpid");
	if(!WIFEXITED(status) || WEXITSTATUS(status) != 0) THROW_E("removing apparmor profile failed");
}

