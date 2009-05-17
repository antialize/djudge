//-*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
#ifndef __saferun_hh__
#define __saferun_hh__
#include <cstring>

//Runs an executable in a safe manner, returns run result values
int saferun(int in, //Use this fd as stdin
	    int out,  //Use this fd as stdout
	    int error, //Used this fd as stderr,
	    size_t memoryLimit, //Memory limit in bytes
	    size_t outputLimit, //Output limit in bytes
	    int user, //Drop to this user when running
		int group, 
	    float & time, //The maximum time allowed to run, opun return the actual time speend running.
	    const char * program, //The name of the program to run
	    ... //NULL termiated list of arguments
	);
#endif //__saferun_hh__
