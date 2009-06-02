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

Unlock::Unlock() {pthread_mutex_unlock(&bigLock);}
Unlock::~Unlock() {pthread_mutex_lock(&bigLock);}
Cond::Cond() {pthread_cond_init(&c, NULL);}
Cond::~Cond() {pthread_cond_destroy(&c);}
void Cond::signal() {pthread_cond_signal(&c);}
void Cond::wait() {pthread_cond_wait(&c,&bigLock);}
void Cond::timedWait(int sec) {
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	t.tv_sec += sec;
	pthread_cond_timedwait(&c, &bigLock, &t);
}
BigLock::BigLock() {
	pthread_mutex_init(&bigLock,NULL);
	pthread_mutex_lock(&bigLock);
}
BigLock::~BigLock() {
	pthread_mutex_unlock(&bigLock);
	pthread_mutex_destroy(&bigLock);
}
pthread_mutex_t bigLock;
