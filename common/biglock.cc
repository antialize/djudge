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
#include "biglock.hh"
#include <iostream>
using namespace std;

#define DEBUG_LOG
#ifdef DEBUG_LOG
#define __LOCK__
#else
#define __LOCK__ if(false)
#endif

Unlock::Unlock() {
	__LOCK__ cout << "Unlock " << pthread_self()%3001 << endl;
	pthread_mutex_unlock(&bigLock);
}

Unlock::~Unlock() {
	__LOCK__ cout << "Lock " << pthread_self()%3001 << endl;
	pthread_mutex_lock(&bigLock);
	__LOCK__ cout << "Running " << pthread_self()%3001 << endl;
}

Lock::Lock() {
	__LOCK__ cout << "Lock " << pthread_self()%3001 << endl;
	pthread_mutex_lock(&bigLock);
	__LOCK__ cout << "Running " << pthread_self()%3001 << endl;
}

Lock::~Lock() {
	__LOCK__ cout << "Unlock " << pthread_self()%3001 << endl;
	pthread_mutex_unlock(&bigLock);
}

Cond::Cond() {pthread_cond_init(&c, NULL);}
Cond::~Cond() {pthread_cond_destroy(&c);}
void Cond::signal() {pthread_cond_signal(&c);}
void Cond::wait() {
	__LOCK__ cout << "Wait " << pthread_self()%3001 << endl;	
	pthread_cond_wait(&c,&bigLock);
	__LOCK__ cout << "Running " << pthread_self()%3001 << endl;
}
void Cond::timedWait(int sec) {
	__LOCK__ cout << "Wait " << pthread_self()%3001 << endl;	
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	t.tv_sec += sec;
	pthread_cond_timedwait(&c, &bigLock, &t);
	__LOCK__ cout << "Running " << pthread_self()%3001 << endl;
}
BigLock::BigLock() {
	pthread_mutex_init(&bigLock,NULL);
	pthread_mutex_lock(&bigLock);
	__LOCK__ cout << "Running " << pthread_self()%3001 << endl;
}
BigLock::~BigLock() {
	pthread_mutex_unlock(&bigLock);
	pthread_mutex_destroy(&bigLock);
}
pthread_mutex_t bigLock;
