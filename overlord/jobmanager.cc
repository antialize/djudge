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

pthread_mutex_t JobManager::mutex;
pthread_cond_t JobManager::jobCond;
pthread_cond_t JobManager::droneCond;
deque<Drone *> JobManager::freeDrones;
deque<Job *> JobManager::jobQueue;
set<Drone *> JobManager::drones;
uint64_t JobManager::idc;

void JobManager::init() {
	idc=0;
	pthread_mutex_init(&mutex,NULL);
	pthread_cond_init(&jobCond,NULL);
	pthread_cond_init(&droneCond,NULL);
}

uint64_t JobManager::addJob(Job * i) {
	pthread_mutex_lock(&mutex);
	uint64_t _=i->id;
	if(i->type == push || i->type == dispose) {
		for(std::set<Drone *>::iterator _ = drones.begin(); _ != drones.end(); ++_) {
			Job * j = new Job();
			j->type = i->type;
			j->id = 0;
			j->client = NULL;
			j->a = i->a;
			j->b = i->b;
			j->c = i->c;
			(*_)->addJob(j);
		}		
		delete i;
	} else {
		cout << "JobManager: Added job " << i->id << " to the job queue" << endl;
		jobQueue.push_back(i);
		pthread_cond_signal(&jobCond);
	}
	pthread_mutex_unlock(&mutex);
	return _;
}

uint64_t JobManager::addJob(JobType type, Client * client, const std::string & a, 
							const std::string & b, const std::string & c) {
	Job * j = new Job();
	pthread_mutex_lock(&mutex);
	j->id = idc++;
	pthread_mutex_unlock(&mutex);
	j->type = type;
	j->client = client;
	j->a = a;
	j->b = b;
	j->c = c;
	return addJob(j);
}

void JobManager::registerDrone(Drone *d) {
	pthread_mutex_lock(&mutex);
	drones.insert(d);
	freeDrones.push_back(d);
	pthread_cond_signal(&droneCond);
	pthread_mutex_unlock(&mutex);
}


void JobManager::unregisterDrone(Drone *d) {
	pthread_mutex_lock(&mutex);
	drones.erase(d);
	for(std::deque<Drone *>::iterator i = freeDrones.begin(); i != freeDrones.end(); ++i) {
		if(*i != d) continue;
		freeDrones.erase(i);
		break;
	}
	pthread_mutex_unlock(&mutex);
}


void JobManager::freeDrone(Drone * d) {
	pthread_mutex_lock(&mutex);
	freeDrones.push_back(d);
	pthread_cond_signal(&droneCond);
	pthread_mutex_unlock(&mutex);
}

void JobManager::run() {
	pthread_mutex_lock(&mutex);
	while(true) {
		while(jobQueue.empty()) pthread_cond_wait(&jobCond, &mutex);
		Job * j = jobQueue.front();
		jobQueue.pop_front();
		while(freeDrones.empty()) pthread_cond_wait(&droneCond, &mutex);
		Drone * d = freeDrones.front();
		freeDrones.pop_front();
		pthread_mutex_unlock(&mutex);
		cout << "Jobmanager: Pushing job " << j->id << " to " << d << endl;
		d->addJob(j);
		pthread_mutex_lock(&mutex);
	}
	pthread_mutex_unlock(&mutex);
}
