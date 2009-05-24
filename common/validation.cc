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
#include "validation.hh"
using namespace std;
#include <iostream>

bool validateEntryName(const std::string & name) {
	if(name.size() < 3) return false;
	if(name.size() >= ENTRY_NAME_LENGTH) return false;
	for(size_t i=0; i < name.size(); ++i) 
		if((name[i] < 'a' || 'z' < name[i]) &&
		   (name[i] < 'A' || 'Z' < name[i]) &&
		   (name[i] < '0' || '9' < name[i]) &&
		   name[i] != '_') return false;
	return true;
}

