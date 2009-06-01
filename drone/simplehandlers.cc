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
#include "error.hh"
#include "fs.hh"
#include "globals.hh"
#include "results.hh"
#include "validation.hh"
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <sys/wait.h>

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
		DIR * d = opendir(entriesPath.c_str());
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
		std::string name = s.readString(ENTRY_NAME_LENGTH);
		if(!validateEntryName(name)) {
			s.write(XSTR(RUN_INVALID_ENTRY));
			s.write("invalid name");
			return;
		}
		std::string p=entriesPath + "/" + name;
		rmrf(p.c_str());
		s.write(XSTR(RUN_SUCCESS));
		s.write("success");
	};
};
CommandHandler * produceDestroyHandler() {return new DestroyHandler();}
