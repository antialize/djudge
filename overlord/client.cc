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
#include "error.hh"
#include "globals.hh"
#include "jobmanager.hh"
#include "results.hh"
#include "validation.hh"
#include <iostream>
#include <sstream>
#include <stdlib.h>
using namespace std;
std::map<std::string, ASyncClient *> ASyncClient::cookieMap;

bool Client::handleCommand(const std::string & cmd, PackageSocket & s) {
	if(cmd == "status") {
		ostringstream res;
		res << "The overlord is up and running" << endl
			<< "Drones:       " << JobManager::drones.size() << endl
			<< "Free:         " << JobManager::freeDrones.size() << endl
			<< "Pending jobs: " << JobManager::jobQueue.size();
		{UNLOCK;
			s.write(XSTR(RUN_SUCCESS));
			s.write(res.str());
		}
	} else if(cmd == "" || cmd == "leave")
		return false;
	else if(cmd == "identify") {
		//TODO implement me
		{UNLOCK;
			string name=s.readString(1024);
			string password=s.readString(1024);
			s.write(XSTR(RUN_SUCCESS));
			s.write("success");
		}
	} else if(cmd == "list") {
		std::set<std::string> e(entries.begin(),entries.end());
		{UNLOCK;
			for(std::set<std::string>::iterator i=e.begin(); i != e.end(); ++i)
				s.write((*i));
			s.write("");
		}
	} else if(cmd == "dispose") {
		string name;
		{UNLOCK;
			name=s.readString(ENTRY_NAME_LENGTH);
			if(!validateEntryName(name)) {
				s.write(XSTR(RUN_INVALID_ENTRY));
				s.write("Invalid entry name");
				return true;
			}
		}
		entries.erase(name);
		JobManager::addJob(dispose,this,name);
		unlink((entriesPath+"/"+name).c_str());		
		{UNLOCK;
			s.write(XSTR(RUN_SUCCESS));
			s.write("Lazily disposing entry");
		}
	} else if(cmd == "push") {
		string name;
		{UNLOCK;
			name=s.readString(ENTRY_NAME_LENGTH);
			if(!validateEntryName(name)) {
				s.write(XSTR(RUN_INVALID_ENTRY));
				s.write("Invalid entry name");
				return true;
			}
		}
		bool found=entries.count(name)>0;
		if(!found) {
			{UNLOCK;
				s.write(XSTR(RUN_INVALID_ENTRY));
				s.write("The entry does not exist");
			}
			return true;
		}
		JobManager::addJob(push,this,name);
		{UNLOCK;
			s.write(XSTR(RUN_SUCCESS));
			s.write("Lasily pushing entry");
		}
	} else if(cmd == "judge") {
		string name,lang;
		{UNLOCK;
			name=s.readString(ENTRY_NAME_LENGTH);
			lang=s.readString(LANG_NAME_LENGTH);
		}
		char buff[64];
		strcpy(buff,"/tmp/codeXXXXXX");
		int f = mkstemp(buff);
		if(f == -1) THROW_PE("mkstemp() failed");
		ULCALL(s.readFD(f));
		close(f);
		bool found=validateEntryName(name) && entries.count(name)>0;
		if(!found) {
			{UNLOCK;
				s.write(XSTR(RUN_INVALID_ENTRY));
				s.write("The entry not found");
			}
			unlink(buff);
			return true;
		}
		uint64_t id = JobManager::addJob(judge,this,name,lang,buff);
		jobHook(s,id);
	} else if(cmd == "import") {
		string name=ULCALL(s.readString(ENTRY_NAME_LENGTH));
		char buff[64];
		strcpy(buff,(entriesPath+"/.tmpXXXXXX").c_str());
		int f = mkstemp(buff);
		if(f==-1) THROW_PE("mkstemp() failed");
		ULCALL(s.readFD(f));
		close(f);
		if(!validateEntryName(name)) {
			{UNLOCK;
				s.write(XSTR(RUN_INVALID_ENTRY));
				s.write("Invalid entry name");
			}
			unlink(buff);
			return true;
		}
		bool found=entries.count(name)>0;
		if(found) {
			{UNLOCK;
				s.write(XSTR(RUN_INVALID_ENTRY));
				s.write("An entry with that name allredy exists");
			}
			unlink(buff);
			return true;
		} 
		uint64_t id = JobManager::addJob(import,this,name,buff);
		jobHook(s,id);
	} else {
		UNLOCK;
		s.write(XSTR(RUN_BAD_COMMAND));
		s.write("Unknown command "+cmd);
	}
	return true;
}

void Client::run_(PackageSocket & s) {
	while(s.canRead()) {
		string cmd = ULCALL(s.readString(COMMAND_LENGTH));
		if(!handleCommand(cmd,s)) break;
	}
}

void ASyncClient::run(PackageSocket & s) {
	string cookie = ULCALL(s.readString(COOKIE_LENGTH));
	ASyncClient * c;
	if(cookieMap.count(cookie) == 0) 
		c = cookieMap[cookie] = new ASyncClient();
	else 
		c = cookieMap[cookie];
	c->run_(s);
}

void ASyncClient::init() {}

void ASyncClient::jobHook(PackageSocket & s, uint64_t jobid) {
	ostringstream r;
	r << jobid;
	{UNLOCK;
		s.write(XSTR(RUN_PENDING));
		s.write(r.str());
	}
}

void ASyncClient::jobDone(Job * j) {
	results.push_back(j);
}

bool ASyncClient::handleCommand(const std::string & cmd, PackageSocket & s) {
	if(cmd == "pullresults") {
		std::vector<Job*> res(results.begin(), results.end());
		results.clear();
		{UNLOCK;
			for(size_t i=0; i < res.size(); ++i) {
				ostringstream id,r;
				id << res[i]->id;
				r << res[i]->result;
				s.write(id.str());
				s.write(r.str());
				s.write(results[i]->msg);
				delete(results[i]);
			}
			s.write("");
		}
		return true;
	} else
		return Client::handleCommand(cmd,s);
}

SyncClient::SyncClient() {
	job=NULL;
}


void SyncClient::run(PackageSocket & s) {
	SyncClient c;
	c.run_(s);
}

void SyncClient::jobHook(PackageSocket & s, uint64_t) {
	while(job == NULL) resultCond.wait();
	Job * j=job;
	job=NULL;
	ostringstream r;
	r << j->result;
	{UNLOCK;
		s.write(r.str());
		s.write(j->msg);
	}
	delete j;
}

void SyncClient::jobDone(Job * j) {
	job=j;
	resultCond.signal();
}

