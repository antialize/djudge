//-*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
#include "langsupport.hh"
#include "saferun.hh"
#include "results.hh"
using namespace std;

class PythonLangSupport: public LangSupport {
public:
	bool hasSource(std::string name) {
		string path=name+".py";
		return access(path.c_str(),R_OK) == 0;
	}
	int compile(std::string) {return RUN_SUCCESS;}
	int run(std::string name, 
			int in, int out, int err, 
			size_t memoryLimit,
			size_t outputLimit,
			float & time,
			int user) {
		string path=name+".py";
		return saferun(in,out,err,memoryLimit,outputLimit,user,time,"python","python",path.c_str(),NULL);
	}
	float rank() {return 99;}
	string name() {return "python";}
};

LangSupport * producePythonLangSupport() {return new PythonLangSupport();}

