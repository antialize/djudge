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

class PtrBase {
public:
	size_t refcount;
	inline PtrBase(): refcount(0) {}
	virtual ~PtrBase() {};
};

template <class T> 
class ptr {
private:
	T * p;
public:
	ptr(): p(NULL) {};
	void set(T * x) {
		if(x == p) return;
		if(p != NULL) {
			p->refcount--;
			//if(p->refcount == 0) delete p;
		}
		p = x;
		if(p) x->refcount++;
	}
	ptr(T * x): p(NULL) {set(x);};
	~ptr() {set(NULL);}
	ptr<T> & operator = (ptr<T> & o) {set(o.p); return *this;}
	ptr<T> & operator = (T * p) {set(p); return *this;}
	bool operator == (const ptr<T> & o) const {return p==o.p;}
	bool operator != (const ptr<T> & o) const {return p!=o.p;}
	bool operator == (const T * o) const {return p==o;}
	bool operator != (const T * o) const {return p!=o;}
	bool operator < (const ptr<T> & o) const {return p<o.p;}
	T * get() {return p;}
	T & operator* () const {return *p;}
	T * operator-> () const {return p;}
};



#endif //__ptr_hh__
