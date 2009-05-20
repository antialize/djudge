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
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <cstring>
#include "packagesocket.hh"
#include <map>
#include "commandhandler.hh"
#include "error.hh"
#include "langsupport.hh"
#include <fstream>
#include <iostream>
#include <boost/program_options.hpp>
#include "globals.hh"
#include <sys/types.h>
#include <pwd.h>
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
	while(ss.canRead()) {
	    //No commant can be more then 1023 chars long
		string command = ss.readString(1024);
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
	int port;
	std::string host;
	std::string droneUserName;
	std::string nobodyUserName;

	po::options_description common("");
	common.add_options()
		("port,p", po::value<int>(&port), "The port of the overlord to connect to.")
		("address,a", po::value<string>(&host), "The adderss of the overload to connect to.")
		("proxyPath", po::value<string>(&proxyPath), "The path to the directory containing the proxy executables.")
		("entriesPath", po::value<string>(&entriesPath), "The path to the entries directory.")
		("droneUser", po::value<string>(&droneUserName), "The username of the drone user.")
		("nobodyUser", po::value<string>(&nobodyUserName), "The username of the nobody user.");
	
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
		if(vm.count("address") == 0) throw po::validation_error("address not specified.");
		if(vm.count("proxyPath") == 0) throw po::validation_error("proxyPath not specified.");
		if(vm.count("entriesPath") == 0) throw po::validation_error("entriesPath not specified.");
		if(vm.count("droneUser") == 0) throw po::validation_error("droneUser not specified.");
		if(vm.count("nobodyUser") == 0) throw po::validation_error("nobodyUser not specified.");
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
