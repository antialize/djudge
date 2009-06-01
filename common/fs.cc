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
#include "error.hh"
#include "fs.hh"
#include <cstring>
#include <dirent.h>
#include <errno.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>

void rmrf(const char * path) {
	if(unlink(path) == -1 && errno == EISDIR) {
		DIR * d = opendir(path);
		if(d != NULL) {
			while(struct dirent * e = readdir(d)) {
				if(strcmp(e->d_name,".") != 0 && strcmp(e->d_name,"..") != 0) {
					std::string p = path;
					p += "/";
					p += e->d_name;
					rmrf(p.c_str());
				}
			}
		}
		rmdir(path);
	}
}

std::string getCWD() {
	char cwd[1025];
	if(getcwd(cwd,1024) == NULL) THROW_PE("getcwd() failed");
	cwd[1024]='\0';
	return cwd;
}

std::string absolutePath(std::string relative) {
	if(relative[0] == '/') return relative;
	return getCWD() + "/" + relative;
}
