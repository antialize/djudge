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
#ifndef __jobmanager_hh__
#define __jobmanager_hh__
#include "biglock.hh"
#include <deque>
#include <string>
#include <set>

enum JobType {
	import,
	judge,
	dispose,
	push,
};

class Drone;
class Client;

struct Job {
	JobType type;
	uint64_t id;
	Client * client;
	std::string a,b,c;
	int result;
	std::string msg; 
};

class JobManager {
private:
	static Cond jobCond;
	static Cond droneCond;
	static uint64_t idc;
public:
	static std::deque<Drone *> freeDrones;
	static std::deque<Job *> jobQueue;
	static std::set<Drone *> drones;
	static void init();
	static uint64_t addJob(Job * j);
	static uint64_t addJob(JobType type, Client * c, const std::string & payload1, const std::string & payload2="", const std::string & payload3="");  
	static void freeDrone(Drone * d);
	static void registerDrone(Drone *d);
	static void unregisterDrone(Drone *d);
	static void run();
};

#endif //__jobmanager_hh__
