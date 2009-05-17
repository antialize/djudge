//-*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
#include "langsupport.hh"
#include "saferun.hh"
#include "results.hh"
#include "apparmor.hh"
#include "globals.hh"
using namespace std;

class CCLangSupport: public LangSupport {
public:
	bool hasSource(std::string name) {
		string path=name+".cc";
		return access(path.c_str(),R_OK) == 0;
	}
	float rank() {return 1;}
	string name() {return "cc";}
	
	void restrictRun(std::string name ,bool entryAccess) {
		char cwd[1025];
		getcwd(cwd,1024);
		char buff[2048];
		buff[0] = '\0';
		if(entryAccess) sprintf(buff,"  %s/**    rw,\n",cwd);
		appArmorLoadProfile(
			"#include <tunables/global>\n"
			"%s/%s.out {\n"
			"  #include <abstractions/base>\n"
			"  %s/%s.out                 r,\n"
			"  %s"
			"}\n",
			cwd,name.c_str(),
			cwd,name.c_str(),
			buff);
	}
			
	void unrestrictRun(std::string name) {
		char cwd[1025];
		getcwd(cwd,1024);
		char buff[2048];
		sprintf(buff,"%s/%s.out",cwd,name.c_str());
		appArmorRemoveProfile(buff);
	}

	int compile(std::string name, int user, int group) {
		string src=name+".cc";
		string dst=name+".out";
		float time=20;
		char cwd[1025];
		getcwd(cwd,1024);
		
		std::string gpp=proxyPath+"/proxy_g++";
		printf("hello\n");
		appArmorLoadProfile(
			"#include <tunables/global>\n"
			"%s {\n"
			"  #include <abstractions/base>\n"
			"  %s                          r,\n"
			"  /tmp/                       rw,\n"
			"  /tmp/**                     rw,\n"
			"  %s/%s                       r,\n"
			"  %s/%s                       rw,\n"
			"  /usr/lib/gcc/*/*/cc1plus    rix,\n"
			"  /usr/lib/gcc/*/*/collect2   rix,\n"
			"  /usr/bin/as                 rix,\n"
			"  /usr/include/**             r,\n"
			"  /usr/bin/ld                 rix,\n"
			"  /usr/bin/nm                 rix,\n"
			"  /usr/bin/strip              rix,\n"
			"  /usr/bin/g++*               rix,\n"
			"}\n",
			gpp.c_str(),gpp.c_str(),
			cwd,src.c_str(),
			cwd,dst.c_str());
		printf("hello\n");
		int x = saferun(0,1,2,100*1024*1024,0,user,group,time,"g++","g++","-O2" , "-Wall" , "-W", "-o", dst.c_str(), src.c_str(), NULL);
		appArmorRemoveProfile(gpp.c_str());
		return x;
	}
	int run(std::string name, 
			int in, int out, int err, 
			size_t memoryLimit,
			size_t outputLimit,
			float & time,
			int user,
			int group
		) {
		string path="./"+name+".out";
		return saferun(in,out,err,memoryLimit,outputLimit,user,group,time,path.c_str(),path.c_str(),NULL);;
	}
};

LangSupport * produceCCLangSupport() {return new CCLangSupport();}
