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
#include "packagesocket.hh"
#include "ptr.hh"
#include <map>
#include <vector>
class Job;

class Client: public PtrBase {
public:
	virtual void jobHook(PackageSocket & s, uint64_t jobid) = 0;
	virtual void jobDone(ptr<Job> & job) = 0;
	virtual bool handleCommand(const std::string & cmd, PackageSocket & s);
	virtual ~Client() {};
	void run_(PackageSocket & s);
};

class ASyncClient: public Client {
private:
	static std::map<std::string, ptr<Client> > cookieMap;
	std::vector<ptr<Job> > results;
public:
	static void run(PackageSocket & s);
	static void init();
	virtual void jobHook(PackageSocket & s, uint64_t jobid);
	virtual void jobDone(ptr<Job> & j);
	virtual bool handleCommand(const std::string & cmd, PackageSocket & s);
};

class SyncClient: public Client {
private:
	Cond resultCond;
	ptr<Job> job;
public:
	SyncClient();
	static void run(PackageSocket & s);
	virtual void jobHook(PackageSocket & s, uint64_t jobid);
	virtual void jobDone(ptr<Job> & j);
};
