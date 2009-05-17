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

