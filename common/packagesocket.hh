//-*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
#ifndef __packagesocket_hh__
#define __packagesocket_hh__
#include <cstring>

class PackageSocket {
private:
    const size_t max_buff_size;
    int sock;
    char * read_buff;
    size_t read_buff_size;
    size_t read_buff_used;
    char * write_buff;
    size_t write_buff_used;
    bool read_escape;
    bool eot;
public:
    PackageSocket(int sock);
    ~PackageSocket();
    bool read(char * b, size_t & len);
    void write(char * b, size_t len=0, bool end_of_package=true);
    bool canRead();
};
#endif //__packagesocket_hh__
