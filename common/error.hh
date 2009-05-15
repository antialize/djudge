//-*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
#ifndef __error_hh__
#define __error_hh__
#include <exception>
#include <cstdarg>
#include <cerrno>
#include <cstring>
#include <cstdio>

class CommonException: public std::exception {
private:
	char buff[2048];
public:
	inline CommonException(int line, const char * file, bool en, const char * format, ...) {
		va_list ap;
		va_start(ap, format);
		int i=snprintf(buff, 2048, "%s: %d\n",file,line);
		if(en && errno) i+=snprintf(buff+i, 2048-i, "%d: %s\n",errno,strerror(errno));
		vsnprintf(buff+i, 2048-i, format, ap);
		va_end(ap);
	}
	const char* what() const throw () {return buff;}
};

#define THROW_PE(FORMAT, ...) throw CommonException(__LINE__,__FILE__,true, FORMAT, ##__VA_ARGS__, NULL)
#define THROW_E(FORMAT, ...) throw CommonException(__LINE__,__FILE__,false, FORMAT, ##__VA_ARGS__, NULL)

#endif//__error_hh__
