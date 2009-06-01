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
#include <sstream>  
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

int Drone::import(PackageSocket & s, const std::string & name, const std::string & path, std::string & msg) {
	s.write("import");
	s.write(name);
	int fd = open(path.c_str(), O_RDONLY);
	if(fd == -1) {
		msg="entry file missing from cache";
		return RUN_INTERNAL_ERROR;
	}
	s.writeFD(fd);
	close(fd);
	string r = s.readString(12).c_str();
	if(r == "" || !s.canRead()) THROW_E("drone died");
	int res = atoi(r.c_str());
	msg = s.readString(1024*128);
	if(res != 0) return res;
	myEntries.insert(name);
	return res;
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
		t.tv_sec += 60;
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
		jobQueue.pop_back();
		pthread_mutex_unlock(&jobQueueLock);
		cout << "Drone " << this << ": Processing job " << j->id << endl;
		try {
			switch(j->type) {
			case ::import:
				j->result = import(s,j->a,j->b, j->msg);
				if(j->result == 0) {
					string path=entriesPath+"/"+j->a;
					if(rename(j->b.c_str(), path.c_str()) == -1) THROW_PE("rename() failed");
					pthread_mutex_lock(&entriesMutex);
					entries.insert(j->a);
					pthread_mutex_unlock(&entriesMutex);
				} else 
					unlink(j->b.c_str());
				break;
			case dispose:
				if(myEntries.count(j->a)) {
					s.write("destroy");
					s.write(j->a);
					string r = s.readString(12).c_str();
					if(r == "" || !s.canRead()) THROW_E("drone died");
					j->result = atoi(r.c_str());
				    j->msg = s.readString(1024*10);
					if(j->result == 0) entries.erase(j->a);
				}
				break;
			case push:
				j->result = import(s,j->a, entriesPath+"/"+j->a, j->msg);
				break;
			case judge:
				if(myEntries.count(j->a) == 0) {
					string msg;
					int res = import(s,j->a, entriesPath+"/"+j->a, msg);
					if(res != 0) {
						j->result = RUN_INTERNAL_ERROR;
						ostringstream s;
						s << "Internal error: import faild (" << res << ")\n" << msg; 
						j->msg = s.str();
						break;
					}
				}
				s.write("judge");
				s.write(j->a); //Send entry name
				s.write(j->b); //Send language
				{
					int f = open(j->c.c_str(),O_RDONLY);
					if(f == -1) THROW_PE("open() failed");
					s.writeFD(f);
					close(f);
					unlink(j->c.c_str());
					string r = s.readString(10).c_str();
					if(r == "" || !s.canRead()) THROW_E("drone died");
					j->result = atoi(r.c_str());
				    j->msg = s.readString(1024*128);
				}
				break;
			default:
				j->result = RUN_INTERNAL_ERROR;
				j->msg = "Not implemented";
			}
			cout << "Drone " << this << ": Finished job " << j->id << "  with result " << j->result << endl;
			if(j->client) 
				j->client->jobDone(j);
			else 
				delete j;
		} catch(std::exception & e) {
			JobManager::addJob(j);
			throw;
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
		throw;
	}
	JobManager::unregisterDrone(&d);
}
