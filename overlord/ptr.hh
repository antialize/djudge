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
#ifndef __ptr_hh__
#define __ptr_hh__
#include <iostream>
#include <sstream>
#include <boost/intrusive_ptr.hpp>

class PtrBase {
public:
	size_t refcount;
	inline PtrBase(): refcount(0) {}
	virtual ~PtrBase() {};
	virtual std::string repr() {
		std::ostringstream s;
		s << this;
		return s.str();
	}
};

template <class T>
class ptr: public boost::intrusive_ptr<T> {
public:
	ptr(): boost::intrusive_ptr<T>() {};
	ptr(T * b,bool addref=true): boost::intrusive_ptr<T>(b,addref) {};
};

inline void intrusive_ptr_release(PtrBase * x) {
	x->refcount--;
	if(x->refcount == 0) delete x;
}

inline void intrusive_ptr_add_ref(PtrBase * x) {
	x->refcount++;
}

#endif //__ptr_hh__
