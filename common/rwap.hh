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
#ifndef __rwap_hh__
#define __rwap_hh_
#include <sys/types.h>
#include <dirent.h>

class sfile {
public:
  FILE * f;
  inline sfile(FILE *_=NULL): f(_) {};
  inline void set(FILE *_) {if(f != NULL) fclose(f); f=_;}
  inline void close() {set(NULL);}
  inline ~sfile() {close();}
  inline operator FILE *() {return f;}
};

class file {
public:
  int fd;
  inline file(int _=-1): fd(_) {};
  inline void set(int _) {if(fd != -1) ::close(fd); fd=_;}
  inline void close() {set(-1);}
  inline ~file() {close();}
  //  inline int operator=(int _) {set(_);return _;}
  inline  operator int() {return fd;}
};

class dir {
public:
  DIR * d;
  inline dir(DIR * _=NULL): d(_) {};
  inline void set(DIR * _) {if(d != NULL) closedir(d); d=_;}
  inline void close() {set(NULL);}
  inline ~dir() {close();}
  inline operator DIR*() {return d;}
};

#endif //__rwap_hh__
