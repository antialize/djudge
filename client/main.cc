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
#include "error.hh"
#include "packagesocket.hh"
#include <boost/program_options.hpp>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <stdio.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cctype>
#include <algorithm>
#include "results.hh"

namespace po = boost::program_options;
using namespace std;


int tolower_(int _) {return tolower(_);}

int main(int argc, char ** argv) {
	string user;
	string password;
	int port = 13049;
	string address = "localhost";
	string command;
	string language;
	string entry;
	string input;
	
	po::options_description cmdline("Drone overload");
	po::positional_options_description pd;
	cmdline.add_options()
		("help,h", "Display this message.")
		("address,a", po::value<string>(&address), "The address of the overlord to connect to")
		("port,p", po::value<int>(&port), "The port of the overlord to connect to")
		("user,u", po::value<string>(&user), "The username for restricted functions")
		("password,P", po::value<string>(&password), "The password for restricted functions")
		("command,c", po::value<string>(&command), "The command to issue: dispose, import, push, status, judge, list")
		("language,l", po::value<string>(&language), "The language of the program to be judged: cc, python, java")
		("entry,e", po::value<string>(&entry), "")
		("input,i", po::value<string>(&input), "");
	pd.add("command",1);
	pd.add("entry",2);
	//pd.add("input",3);
	//pd.add("language",4);
	try {
		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).
				  options(cmdline).positional(pd).run(), vm);;
		po::notify(vm);
		if(vm.count("command") == 0) throw po::validation_error("command not specified");
		if(command != "dispose" &&
		   command != "import" && 
		   command != "push" && 
		   command != "status" &&
		   command != "list" &&
		   command != "judge") throw po::validation_error("invalid command");
		if(command != "list" && command != "status" && command != "import" && vm.count("entry")==0) 
			throw po::validation_error("entry not specified");
		if(command == "import" && input=="") input=entry;
		if(command == "import" && input=="") throw po::validation_error("No input specified");
		if(vm.count("help")) {
			cout << cmdline  << endl;
			exit(0);
		}
	} catch(std::exception & e) {
		cerr << "Argument error: " << e.what() << endl;
		cerr << cmdline << endl;
		exit(1);
	}
	try {
		if(false) { //command != "judge" && command != "list") {
			char buff[1024];
			if(user == "" ) {
				printf("username: ");
				fflush(stdout);
				fgets(buff, 1023, stdin);
				buff[1023] = '\0';
				user=buff;
				if(user == "") exit(0);
			}
			if(password == "") {
				password = getpass("password: ");
				if(password == "") exit(0);
			}
		}

		int s = socket(AF_INET,SOCK_STREAM,0);
		if(s == -1) THROW_PE("socket() failed\n");
		struct hostent *host;
		host = gethostbyname(address.c_str());
		struct sockaddr_in server_addr;  
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(port);   
		server_addr.sin_addr = *((struct in_addr *)host->h_addr);
		if(connect(s, (struct sockaddr *)&server_addr,  sizeof(struct sockaddr)) == -1)
			THROW_PE("connect() failed\n");
		PackageSocket ss(s);
		ss.write("syncclient");
		string r = ss.readString(1024);
		if(r != "success") {
			cerr << "Unexpected responce: " << r << endl;
			exit(1);
		}
		if(user != "") {
			ss.write("identify");
			ss.write(user.c_str());
			ss.write(password.c_str());
			string x = ss.readString(32);
			string r = ss.readString(1024);
			if(x != XSTR(RUN_SUCCESS)) {
				cerr << "Username or password invalid: " << r << endl;
				exit(1);
			}
		}
		if(command == "status") {
			ss.write("status");
			string a = ss.readString(10); 
			cout << a << endl << ss.readString(1024*128) << endl;
		} else if(command == "list") {
			ss.write("list");
			while(true) {
				string x = ss.readString(1024);
				if(x == "") break;
				cout << x << endl;
			}
		} else if(command == "dispose") {
			ss.write("dispose");
			ss.write(entry);
			string a = ss.readString(10); 
			cout << a << endl << ss.readString(1024) << endl;
		} else if(command == "push") {
			ss.write("push");
			ss.write(entry);
			string a = ss.readString(10);
			cout << a << endl << ss.readString(1024) << endl;
		} else if(command == "judge") {
			if(language == "") {
				size_t r = input.rfind('.');
				string ext = input.substr(r+1);
				transform(ext.begin(),ext.end(),ext.begin(), tolower_);
				if(ext == "py")
					language = "python";
				else if(ext == "java")
					language = "java";
				else
					language = "cc";
			}
			ss.write("judge");
			ss.write(entry);
			ss.write(language);
			int f = open(input.c_str(),O_RDONLY);
			if(f == -1) THROW_PE("open() failed");
			ss.writeFD(f);
			close(f);
			string a = ss.readString(10); 
			cout << a  << endl << ss.readString(1024*128) << endl;
		} else if(command == "import") {
			size_t l = input.rfind('/');
			if(l == string::npos) l=-1;
			string x=input.substr(l+1);
			size_t r = x.find('.');
			if(r == string::npos) {
				cerr << "Input seems to not be a tar file";
				exit(1);
			}
			int f = open(input.c_str(),O_RDONLY);
			if(f == -1) THROW_PE("open() failed");
			entry = x.substr(0,r);
			ss.write("import");
			ss.write(entry);
			ss.writeFD(f);
			close(f);
			string a = ss.readString(10); 
			cout << a << endl << ss.readString(1024*128) << endl;
		}
		ss.write("leave");
		close(s);
	} catch(std::exception & e) {
		cerr << "Exception: " << e.what() << endl;
	};
}
