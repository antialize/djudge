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
#ifndef __langsupport_hh__
#define __langsupport_hh__
#include <string>
#include <map>

class LangSupport {
public:
	virtual bool hasSource(std::string name) = 0;
	virtual int compile(std::string name, int user, int group) = 0;
	virtual void restrictRun(std::string name, bool entryAccess) = 0;
	virtual void unrestrictRun(std::string name) = 0;
	virtual int run(std::string name, 
					int in, int out, int err, 
					size_t memoryLimit,
					size_t outputLimit,
					float & time,			   
					int user,
		            int group) = 0;
	virtual float rank() = 0;
	virtual std::string name() = 0;
};
typedef std::multimap<float, LangSupport *> langByRank_t;

extern langByRank_t langByRank;
extern std::map<std::string, LangSupport *> langByName;


LangSupport * producePythonLangSupport();
LangSupport * produceCCLangSupport();
LangSupport * produceJavaLangSupport();


#endif //__langsupport_hh__
