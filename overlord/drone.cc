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
#include "globals.hh"
#include "results.hh"
#include "validation.hh"
#include <fcntl.h>
#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
using namespace std;

Drone::Drone() {
	pthread_mutex_init(&jobQueueLock, NULL);
	pthread_cond_init(&jobQueueCond, NULL);
}

Drone::~Drone() {
	pthread_cond_destroy(&jobQueueCond);
	pthread_mutex_destroy(&jobQueueLock);
}

void Drone::addJob(Job * job) {
	pthread_mutex_lock(&jobQueueLock);
	jobQueue.push_back(job);
	pthread_cond_signal(&jobQueueCond);
	pthread_mutex_unlock(&jobQueueLock);
}

void Drone::run_(PackageSocket & s) {
	s.write("list");
	while(s.canRead()) {
		string entry = s.readString(ENTRY_NAME_LENGTH);
		if(entry == "") break;
		myEntries.insert(entry);
	}
	pthread_mutex_lock(&jobQueueLock);
	while(s.canRead()) {
		struct timespec t;
		clock_gettime(CLOCK_REALTIME, &t);
		t.tv_sec += 5;
		pthread_cond_timedwait(&jobQueueCond, &jobQueueLock, &t);
		if(jobQueue.empty()) {
			if(!s.canRead()) break;
			pthread_mutex_unlock(&jobQueueLock);
			s.write("ping");
			bool p = (s.readString(1024) == "pong");
			pthread_mutex_lock(&jobQueueLock);
			if(!p) break;
			continue;
		}
		Job * j = jobQueue.back();
		cout << "Drone " << this << ": Processing job " << j->id << endl;
		jobQueue.pop_back();
		pthread_mutex_unlock(&jobQueueLock);
		try {
			switch(j->type) {
			case import:
			{
				s.write("import");
				s.write(j->a);
				int fd = open(j->b.c_str(), O_RDONLY);
				s.writeFD(fd);
				string r = s.readString(10).c_str();
				if(r == "" || !s.canRead()) THROW_E("drone died");
				int res = atoi(r.c_str());
				string msg = s.readString(1024*10);
				if(res == 0) {
					string path=entriesPath+"/"+j->a;
					if(rename(j->b.c_str(), path.c_str()) == -1) {
						res=RUN_INTERNAL_ERROR;
						msg="Overload: Rename failed";
					}
				}
				if(res == 0) {
					myEntries.insert(j->a);
					pthread_mutex_lock(&entriesMutex);
					entries.insert(j->a);
					pthread_mutex_unlock(&entriesMutex);
				} else
					unlink(j->b.c_str());
				j->result = res;
				j->msg = msg;
			}
			break;
			case dispose:
				if(myEntries.count(j->a)) {
					s.write("destroy");
					s.write(j->a);
					string r = s.readString(10).c_str();
					if(r == "" || !s.canRead()) THROW_E("drone died");
					j->result = atoi(r.c_str());
				    j->msg = s.readString(128);
					if(j->result == 0) entries.erase(j->a);
				}
				break;
			case judge:
			case push:
			default:
				j->result = RUN_INTERNAL_ERROR;
				j->msg = "Not implemented";
				cout << j->type << endl;
			}
			cout << "Drone " << this << ": Finished job " << j->id << "  with result " << j->result << endl;
			if(j->client) 
				j->client->jobDone(j);
			else 
				delete j;
			//Forward the job					
		} catch(std::exception & e) {
			JobManager::addJob(j);
			throw e;
		}
		pthread_mutex_lock(&jobQueueLock);
		if(jobQueue.empty()) JobManager::freeDrone(this);
	}
	pthread_mutex_unlock(&jobQueueLock);
}

void Drone::run(PackageSocket & s) {
	Drone d;
	JobManager::registerDrone(&d);
	try {
		d.run_(s);
	} catch(std::exception & e) {
		JobManager::unregisterDrone(&d);
		throw e;
	}
	JobManager::unregisterDrone(&d);
}
