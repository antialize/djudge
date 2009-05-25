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
#include "packagesocket.hh"
#include "string.h"
#include <arpa/inet.h>
#include <boost/program_options.hpp>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/types.h>
#include <unistd.h>
namespace po = boost::program_options;
using namespace std;

int s;
void handler(int) {
	cerr << "Overlord Parachute Deployed" << endl;
	close(s);
	exit(1);
};

void* dispatch(void * _) {
	ptrdiff_t cs = reinterpret_cast<ptrdiff_t>(_);
	printf("Connect\n");
	try {
		PackageSocket ss(cs);
		string type=ss.readString(20);
		if(type == "syncclient") {
			ss.write("success");
			SyncClient::run(ss);
		} else if(type == "asyncclient") {
			ss.write("success");
			ASyncClient::run(ss);
		} else if(type == "drone") {
			ss.write("success");
			Drone::run(ss);
		} else {
			ss.write("invalid type");
		}
	} catch(std::exception& e) {
		cerr << "Exception: " << e.what() << endl;
	}
	close(cs);
	return NULL;
}

void runServer(int port) {
// 	signal(SIGHUP, handler);
// 	signal(SIGINT, handler);
// 	signal(SIGQUIT, handler);
// 	signal(SIGILL, handler);
// 	signal(SIGABRT, handler);
// 	signal(SIGFPE, handler);
// 	signal(SIGSEGV, handler);
 	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, handler);
	sockaddr_in a;
	s = socket(AF_INET,SOCK_STREAM,0);
	a.sin_family = AF_INET;
	a.sin_addr.s_addr = INADDR_ANY;
	a.sin_port = htons(port);
	if( bind(s,(struct sockaddr*)&a, sizeof(a)) == -1) THROW_PE("bind() failed");
	listen(s, 128);
	while(true) {
		socklen_t _=sizeof(a);
		ptrdiff_t ss =accept(s,(struct sockaddr*)&a,&_);
		pthread_t thread;
		pthread_create(&thread,NULL, dispatch, reinterpret_cast<void *>(ss));
	}
};

void * jobManagerMain(void *) {
	JobManager::run();
	return NULL;
}

int main(int argc, char ** argv) {
	int port=13049;
	po::options_description cmdline("Drone overload");
	cmdline.add_options() 
		("help,h", "Display the message.")
		("port,p", po::value<int>(&port), "The port to bind to.");
   	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc,argv, cmdline), vm);
		po::notify(vm);
		if (vm.count("help")) {
			cout << cmdline << endl;
			exit(0);
		}
	} catch(std::exception & e) {
		cerr << "Argument error: " << e.what() << endl;
		cerr << cmdline << endl;
		exit(1);
	}

	entriesPath="overlordEntries";
	DIR * d = opendir(entriesPath.c_str());
	if(d == NULL) THROW_PE("opendir()");
	while(struct dirent * e=readdir(d)) {
		if(e->d_name[0] == '.' || e->d_name[0] == '\0') continue;
		entries.insert(e->d_name);
	}
	closedir(d);
	pthread_mutex_init(&entriesMutex,NULL);

	ASyncClient::init();
	JobManager::init();
	pthread_t thread;
	pthread_create(&thread,NULL, jobManagerMain, NULL);
	runServer(port);
	return 0;
}
