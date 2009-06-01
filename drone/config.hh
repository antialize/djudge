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
#ifndef __config_hh__
#define __config_hh__
#include <string>
#include <map>
#include <set>
#include <stdint.h>

class Config {
private:
	bool dfi;
	std::set<std::string> langs;
	std::map<std::string, float> base;
	std::map<std::string, float> scale;
	std::map<std::string, uint64_t> memory;
	uint64_t output;
	friend Config * parseConfig(std::string file);
public:
	uint64_t memoryLimit(const std::string & lang) const;
	uint64_t outputLimit() const;
	float timeBase(const std::string & lang) const;
	float timeScale(const std::string & lang) const;
	bool isLangAllowed(const std::string & lang) const;
	bool displayFailedInput() const;
};

Config * parseConfig(std::string file);
#endif //__config_hh__
