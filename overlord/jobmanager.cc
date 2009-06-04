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
#include "client.hh"
#include "drone.hh"
#include "error.hh"
#include "jobmanager.hh"
#include <iostream>
using namespace std;

Cond JobManager::jobCond;
Cond JobManager::droneCond;
deque<ptr<Drone> > JobManager::freeDrones;
deque<ptr<Job> > JobManager::jobQueue;
set<ptr<Drone> > JobManager::drones;
uint64_t JobManager::idc;

void JobManager::init() {
	idc=0;
}

uint64_t JobManager::addJob(ptr<Job> & i) {
	uint64_t _=i->id;
	if(i->type == push || i->type == dispose) {
		for(std::set<ptr<Drone> >::iterator _ = drones.begin(); _ != drones.end(); ++_) {
			ptr<Job> j = new Job();
			j->type = i->type;
			j->id = 0;
			j->client = NULL;
			j->a = i->a;
			j->b = i->b;
			j->c = i->c;
			(*_)->addJob(j);
		}		
	} else {
		cout << "JobManager: Added job " << i->id << " to the job queue" << endl;
		jobQueue.push_back(i);
		jobCond.signal();
	}
	return _;
}

uint64_t JobManager::addJob(JobType type, ptr<Client> & client, const std::string & a, 
							const std::string & b, const std::string & c) {
	ptr<Job> j = new Job();
	j->id = idc++;
	j->type = type;
	j->client = client;
	j->a = a;
	j->b = b;
	j->c = c;
	return addJob(j);
}

void JobManager::registerDrone(ptr<Drone> & d) {
	drones.insert(d);
	freeDrones.push_back(d);
	droneCond.signal();
}

void JobManager::unregisterDrone(ptr<Drone> & d) {
	drones.erase(d);
	for(std::deque<ptr<Drone> >::iterator i = freeDrones.begin(); i != freeDrones.end(); ++i) {
		if(*i != d) continue;
		cout << "d unregister drone Hay " << d.get() << endl;
		freeDrones.erase(i);
		break;
	}
	cout << "f unregister drone " << d.get() << endl;
}


void JobManager::freeDrone(ptr<Drone> & d) {
	freeDrones.push_back(d);
	droneCond.signal();
}

void JobManager::run() {
	while(true) {
		while(jobQueue.empty()) jobCond.wait();
		ptr<Job> j = jobQueue.front();
		jobQueue.pop_front();
		while(freeDrones.empty()) droneCond.wait();
		ptr<Drone> d = freeDrones.front();
		freeDrones.pop_front();
		cout << "Jobmanager: Pushing job " << j->id << " to " << d.get() << endl;
		d->addJob(j);
		cout << "Push done" << endl;
	}
}
