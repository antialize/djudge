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
#include <boost/program_options.hpp>
#include <iostream>
namespace po = boost::program_options;

using namespace std;

int main(int argc, char ** argv) {
	int port;
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
};
