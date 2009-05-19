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
#ifndef __error_hh__
#define __error_hh__
#include <exception>
#include <cstdarg>
#include <cerrno>
#include <cstring>
#include <cstdio>

class CommonException: public std::exception {
private:
	char buff[1024*8];
public:
	inline CommonException(int line, const char * file, bool en, const char * format, ...) {
		va_list ap;
		va_start(ap, format);
		int i=snprintf(buff, 1024*8, "%s: %d\n",file,line);
		if(en && errno) i+=snprintf(buff+i, 1024*8-i, "%d: %s\n",errno,strerror(errno));
		vsnprintf(buff+i, 1024*8-i, format, ap);
		va_end(ap);
	}
	const char* what() const throw () {return buff;}
};

#define THROW_PE(FORMAT, ...) throw CommonException(__LINE__,__FILE__,true, FORMAT, ##__VA_ARGS__, NULL)
#define THROW_E(FORMAT, ...) throw CommonException(__LINE__,__FILE__,false, FORMAT, ##__VA_ARGS__, NULL)

#endif//__error_hh__
