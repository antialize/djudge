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
