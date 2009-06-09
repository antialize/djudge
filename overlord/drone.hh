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
#ifndef __drone_hh__
#define __drone_hh__

#include "biglock.hh"
#include "jobmanager.hh"
#include "packagesocket.hh"
#include "ptr.hh"
#include <set>
#include <stdint.h>

class Drone: public PtrBase {
private:
	static uint64_t idc;
	uint64_t id;

	Cond jobQueueCond;
	std::deque<ptr<Job> > jobQueue;
	std::set<std::string> myEntries;
public:
	inline Drone() {id = idc++;}
	int import(PackageSocket & s, const std::string & name, const std::string & path, std::string & msg);
	void addJob(ptr<Job> & job);
	void run_(PackageSocket & s); 
	static void run(PackageSocket & s);
	std::string repr();
};
#endif //__drone_hh__
