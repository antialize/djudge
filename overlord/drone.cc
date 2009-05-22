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
#include "drone.hh"
#include <pthread.h>
#include "error.hh"

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
	pthread_mutex_lock(&jobQueueLock);
	while(true) {
		struct timespec t;
		clock_gettime(CLOCK_REALTIME, &t);
		t.tv_sec += 60;
		pthread_cond_timedwait(&jobQueueCond, &jobQueueLock, &t);
		if(jobQueue.empty()) {
			pthread_mutex_unlock(&jobQueueLock);
			s.write("ping");
			if(s.readString(1024) != "pong") THROW_E("Did not send pong");
			pthread_mutex_lock(&jobQueueLock);
			continue;
		}
		Job * j = jobQueue.back();
		jobQueue.pop_back();
		pthread_mutex_unlock(&jobQueueLock);
		try {
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
	d.run_(s);
}
