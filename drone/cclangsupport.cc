//-*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
#include "langsupport.hh"
#include "saferun.hh"
#include "results.hh"
using namespace std;

class CCLangSupport: public LangSupport {
public:
	bool hasSource(std::string name) {
		string path=name+".cc";
		return access(path.c_str(),R_OK) == 0;
	}
	float rank() {return 1;}
	string name() {return "cc";}
	int compile(std::string name) {
		string src=name+".cc";
		string dst=name+".out";
		float time=20;
		int x = saferun(0,1,2,100*1024*1024,0,-2,time,"g++","g++","-O2" , "-Wall" , "-W", "-o", dst.c_str(), src.c_str(), NULL);
		return x;
	}
	int run(std::string name, 
			int in, int out, int err, 
			size_t memoryLimit,
			size_t outputLimit,
			float & time,
			int user) {
		string path="./"+name+".out";
		return saferun(in,out,err,memoryLimit,outputLimit,user,time,path.c_str(),path.c_str(),NULL);;
	}
};

LangSupport * produceCCLangSupport() {return new CCLangSupport();}
