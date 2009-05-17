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
