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
#include "commandhandler.hh"
#include "error.hh"
#include "globals.hh"
#include "langsupport.hh"
#include "packagesocket.hh"
#include "validation.hh"
#include <signal.h>
#include <boost/program_options.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
namespace po = boost::program_options;

using namespace std;

class RequestHandler {
private:
    std::map<std::string, CommandHandler *> handlers;
public:
    void addHandler(CommandHandler * h) {handlers[h->name()] = h;}
    void run(std::string hostName, int port) {
		struct hostent *host;
		int s = socket(AF_INET,SOCK_STREAM,0);
		if(s == -1) THROW_PE("socket() failed\n");
		host = gethostbyname(hostName.c_str());
		struct sockaddr_in server_addr;  
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(port);   
		server_addr.sin_addr = *((struct in_addr *)host->h_addr);
		if(connect(s, (struct sockaddr *)&server_addr,  sizeof(struct sockaddr)) == -1)
			THROW_PE("connect() failed\n");
		PackageSocket ss(s);
		ss.write("drone");
		string r = ss.readString(10);
		if(r != "success") THROW_E("invalid overlord responce: %s,", r.c_str());
		while(ss.canRead()) {
			string command = ss.readString(COMMAND_LENGTH);
			if(command == "quit") break;
			std::map<std::string, CommandHandler *>::iterator i = handlers.find(command);
			if(i == handlers.end()) ss.write("invalid command");
			else i->second->handle(ss);
		}
    };
};

void addLang(LangSupport * l) {
	langByRank.insert( make_pair(l->rank(), l) );
	langByName[ l->name() ] = l;
}

int main(int argc, char ** argv) {
	int port=13049;
	std::string host="127.0.0.1";
	std::string droneUserName="drone";
	std::string nobodyUserName="nobody";
	maxTime = 60*5;
	maxOutput = 1024*1024*1024;
    maxMemory = 256*1024*1024;

 	signal(SIGPIPE, SIG_IGN);
	
	po::options_description common("");
	common.add_options()
		("port,p", po::value<int>(&port), "The port of the overlord to connect to.")
		("address,a", po::value<string>(&host), "The adderss of the overload to connect to.")
		("proxyPath", po::value<string>(&proxyPath), "The path to the directory containing the proxy executables.")
		("entriesPath", po::value<string>(&entriesPath), "The path to the entries directory.")
		("droneUser", po::value<string>(&droneUserName), "The username of the drone user.")
		("nobodyUser", po::value<string>(&nobodyUserName), "The username of the nobody user.")
		("maxTime",po::value<float>(&maxTime), "The maximum time we want a program to run EVER in seconds.")
		("maxOutput",po::value<uint64_t>(&maxOutput), "The maximum number of bytes a program is EVER allowed to output.")
		("maxMemory",po::value<uint64_t>(&maxMemory), "The maximum number of bytes of memory a program is EVER allowd to use.");
	
	po::options_description cmdline("Judge drone client");
	cmdline.add_options()
		("help,h", "Display this message.");
	cmdline.add(common);
 
	po::variables_map vm;
	try {
		std::ifstream cfg;
		cfg.open("~/.djudge/drone");
		po::store(po::parse_command_line(argc,argv, cmdline), vm);
		po::store(po::parse_config_file( cfg, common), vm);
		po::notify(vm);
		if (vm.count("help")) {
			cout << cmdline << endl;
			exit(0);
		}
		if(vm.count("proxyPath") == 0) throw po::validation_error("proxyPath not specified.");
		if(vm.count("entriesPath") == 0) throw po::validation_error("entriesPath not specified.");
	} catch(std::exception & e) {
		cerr << "Argument error: " << e.what() << endl;
		cerr << cmdline << endl;
		exit(1);
	}
	struct passwd * p = getpwnam(droneUserName.c_str());
	if(p == NULL) THROW_PE("getpwnam(%s) failed:", droneUserName.c_str());
	droneUser = p->pw_uid;
	droneGroup = p->pw_gid;
	p = getpwnam(nobodyUserName.c_str());
	if(p == NULL) THROW_PE("getpwnam(%s) failed:", nobodyUserName.c_str());
	nobodyUser = p->pw_uid;
	nobodyGroup = p->pw_gid;
	addLang(produceCCLangSupport());
	addLang(producePythonLangSupport());

    RequestHandler r;
    r.addHandler(produceVersionHandler());
    r.addHandler(producePingHandler());
    r.addHandler(produceListHandler());
    r.addHandler(produceImportHandler());
    r.addHandler(produceDestroyHandler());
	r.addHandler(produceJudgeHandler());
    r.run(host.c_str(),port);
}
