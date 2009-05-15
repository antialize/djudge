//-*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
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
	char buff[1024];
	while(ss.canRead()) {
	    //No commant can be more then 1023 chars long
	    size_t x=1023;
	    if(!ss.read(buff,x)) continue;
	    buff[x] = '\0';
	    if(!strcmp(buff,"quit")) break;
	    std::map<std::string, CommandHandler *>::iterator i = handlers.find(buff);
	    if(i == handlers.end()) ss.write("invalid command");
	    else i->second->handle(ss);
	}
    };
};

int main(int argc, char ** argv) {
    RequestHandler r;
    r.addHandler(produceVersionHandler());
    r.addHandler(producePingHandler());
    r.addHandler(produceListHandler());
    r.addHandler(produceImportHandler());
    r.addHandler(produceDestroyHandler());
    //r.addHandler(produceJudgeHandler());
    r.run("127.0.0.1",1234);
}
