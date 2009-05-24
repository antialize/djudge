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

#include "jobmanager.hh"
#include "packagesocket.hh"
#include <pthread.h>
#include <set>

class Drone {
private:
	pthread_mutex_t jobQueueLock;
	pthread_cond_t jobQueueCond;
	std::deque<Job *> jobQueue;
	std::set<std::string> myEntries;
public:
	Drone();
	~Drone();
	int import(PackageSocket & s, const std::string & name, const std::string & path, std::string & msg);
	
	void addJob(Job * job);
	void run_(PackageSocket & s); 
	static void run(PackageSocket & s);
};
#endif //__drone_hh__
