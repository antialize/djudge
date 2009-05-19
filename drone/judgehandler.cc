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

using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "error.hh"
#include "langsupport.hh"
#include "globals.hh"
#include "results.hh"

class JudgeHandler: public CommandHandler {
public:
	std::string name() const {return "judge";}
	void handle(PackageSocket & s) {
		try {
			umask(~S_IRWXU);
			string entry = s.readString(1024);
			string language = s.readString(1024);
			string verbosity = s.readString(1024);
			char tmpnam[42];
			strcpy(tmpnam,"/tmp/djudge_XXXXXXXX");
			int fd = mkstemp(tmpnam);
			if(fd == -1) THROW_PE("mkstemp() failed");
			if(fchown(fd, nobodyUser, nobodyGroup) == -1) THROW_PE("fchown() failed");
			size_t n=1024*10;
			char buff[n];
			while(!s.read(buff,n)) 
				if(write(fd,buff,n) != (int)n) 
					THROW_PE("write() failed");
			close(fd);
			if(langByName.count(language) == 0) THROW_E("Invalid language");
			LangSupport * l = langByName[language];
			
			if(!l->compile(tmpnam, nobodyUser ,nobodyGroup,s)) return;
			//Open entity
			//Run instances
			//Calculate correct answer

			s.write(XSTR(RUN_SUCCESS));
		} catch(CommonException & e) {
			s.write(XSTR(RUN_INTERNAL_ERROR));
			s.write(e.what());
		}
	}
};
  
