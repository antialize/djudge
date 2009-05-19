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
#include "results.hh"
#include "saferun.hh"
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
#include <sys/wait.h>

static pid_t child;
void timeout(int) {
	kill(child, SIGKILL);
	exit(RUN_TIME_LIMIT_EXCEEDED);
};

int saferun(int in, 
			int out, 
			int err,
			size_t memoryLimit, 
			size_t outputLimit, 
			int user, 
			int group,
			float & time,
			const char * program, 
			... 
	) {
	pid_t outputlimiter=0;
	if(outputLimit > 0) {
		int op[2];
		if(pipe(op) == -1) THROW_PE("pipe() failed\n");
		outputlimiter=fork();
		if(outputlimiter==0) {
			close(op[1]);
			char buff[1024*128];
			size_t r=0;
			while(true) {
				int i=read(op[0],buff,1024*128);
				if(i <= 0) exit(RUN_SUCCESS);
				r+=i;
				if(r > outputLimit) exit(RUN_OUTPUT_LIMIT_EXCEEDED);
				if(write(out,buff,i) != i) exit(RUN_SUCCESS); //TODO: Should we catch this
			}
		}
		if(outputlimiter==-1) THROW_PE("fork() failed\n");
		close(op[0]);
		out=op[1];
	}
	int timep[2];
	if(pipe(timep) == -1) THROW_PE("pipe() failed\n");
	pid_t timer=fork();
	if(timer == 0) {
		close(timep[0]);
		pid_t app=child=fork();
		if(app == 0) {
			close(timep[1]);
			if(in != 0) dup2(in,0);
			if(out != 1) dup2(out,1);
			if(err != 2) dup2(err,2);

			struct rlimit l = {memoryLimit, memoryLimit};
			if(setrlimit(RLIMIT_AS,&l) == -1) THROW_PE("setrlimit() failed\n");			
			setregid(group,group);
			setreuid(user,user);
			va_list ap;
			std::vector<const char *> args;
			va_start(ap, program);
			do {
				args.push_back(va_arg(ap,const char *));
			} while(args.back());
			execvp(program, (char * const *)&args[0]);
			exit(RUN_INTERNAL_ERROR);
		}
		if(app == -1) THROW_PE("fork() failed\n");
		struct itimerval t, cur;
		signal(SIGALRM, timeout);
		t.it_value.tv_sec = time;
		t.it_value.tv_usec = (time - t.it_value.tv_sec)*1000000.0;
		t.it_interval.tv_sec = 0;
		t.it_interval.tv_usec = 0;
		setitimer(ITIMER_REAL, &t, NULL);
		int status;
		waitpid(app, &status, 0);
		getitimer(ITIMER_REAL, &cur);
		if(WIFSIGNALED(status)) exit(RUM_RUNTIME_ERROR);
		if(!WIFEXITED(status)) exit(RUN_ABNORMAL_TERMINATION);
		if(WEXITSTATUS(status) != 0) exit(RUN_EXIT_NOT_ZERO);
		char o[40];
		float time=(t.it_value.tv_sec-cur.it_value.tv_sec) + (t.it_value.tv_usec-cur.it_value.tv_usec)/1000000.0;
		sprintf(o,"%f",time);
		write(timep[1],o,strlen(o));
		close(timep[1]);
		exit(RUN_SUCCESS);		       
	}
	if(outputlimiter != 0) close(out);

	close(timep[1]);
	char i[40];
	int x = read(timep[0], i, 39);
	int status;
	time = -1;
	if(waitpid(timer, &status, 0) == -1) return RUN_INTERNAL_ERROR;
	if(x == -1 || !WIFEXITED(status)) return RUN_INTERNAL_ERROR;
	if(WEXITSTATUS(status) != 0) return WEXITSTATUS(status);
	if(outputlimiter != 0) {
		waitpid(outputlimiter, &status, 0);
		if(!WIFEXITED(status)) return RUN_INTERNAL_ERROR;
		if(WEXITSTATUS(status) != 0) return WEXITSTATUS(status);
	}
	i[x] = '\0';
	sscanf(i,"%f",&time);
	return RUN_SUCCESS;
}
