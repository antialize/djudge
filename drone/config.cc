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
#include "config.hh"
#include "error.hh"
#include "globals.hh"
#include "langsupport.hh"
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

namespace po = boost::program_options;

template <typename T, typename M> 
T fetch(const M & m, const std::string & lang, const T def) {
	T r=def;
	typename M::const_iterator a=m.find("global");
	if(a != m.end()) r=a->second;
	typename M::const_iterator b=m.find(lang);
	if(b != m.end()) r=b->second;
	return r;
}

uint64_t Config::memoryLimit(const std::string & lang) const {return fetch(memory,lang,100);}
uint64_t Config::outputLimit() const {return output;}
float Config::timeBase(const std::string & lang) const {return fetch(base,lang,30.0);}
float Config::timeScale(const std::string & lang) const {return fetch(scale,lang,0.0);}
bool Config::isLangAllowed(const std::string & lang) const {return langs.count(lang);}
bool Config::displayFailedInput() const {return dfi;}

Config * parseConfig(std::string file) {
	po::options_description parse("");
	Config * c = new Config();
	c->dfi = false;
	c->output = 50*1024;
	try {
		std::string langs;
		parse.add_options()
			("displayfailedinput", po::value<bool>(&c->dfi), "")
			("global.displayfailedinput", po::value<bool>(&c->dfi), "")
			("output", po::value<uint64_t>(&c->output),"")
			("global.output", po::value<uint64_t>(&c->output),"")
			("languages", po::value<std::string>(&langs),"")
			("global.languages", po::value<std::string>(&langs),"");
		std::vector<std::string> ls;
		ls.push_back("");
		ls.push_back("global");
		for(langByRank_t::iterator i=langByRank.begin(); i != langByRank.end(); ++i)
			ls.push_back(i->second->name()+"");
		for(std::vector<std::string>::iterator i=ls.begin(); i!=ls.end(); ++i) {
			std::string p=*i;
			if(p != "") p=p+".";
			parse.add_options()
				((p+"memory").c_str(), po::value<uint64_t>() )
				((p+"timebase").c_str(), po::value<float>() )
				((p+"timescale").c_str(), po::value<float>() );
		}
		std::ifstream cfg;
		cfg.open(file.c_str());
		if(!cfg) THROW_E("Unable to open file");
		po::variables_map vm;
		po::store(po::parse_config_file( cfg, parse), vm);
		po::notify(vm);

		//TODO break up langs
		c->langs.insert("all");
		for(std::vector<std::string>::iterator i=ls.begin(); i!=ls.end(); ++i) {
			std::string p=*i;
			std::string d=*i;
			if(p != "") p=p+".";
			if(d == "") d="global";
			if(vm.count(p+"memory")) c->memory[d] = vm[p+"memory"].as<uint64_t>();
			if(vm.count(p+"timebase")) c->base[d] = vm[p+"timebase"].as<float>();
			if(vm.count(p+"timescale")) c->base[d] = vm[p+"timescale"].as<float>();
		}
	} catch(std::exception & e) {
		delete c;
		throw;
	}
	return c;
};
