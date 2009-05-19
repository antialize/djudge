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
#include "packagesocket.hh"
#include "error.hh"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>

PackageSocket::PackageSocket(int _): 
	max_buff_size(1024*128), sock(_), 
	read_buff_size(0), read_buff_used(0), 
	write_buff_used(0), 
	read_escape(false), eot(false) {
	read_buff = new char[max_buff_size];
	write_buff = new char[max_buff_size];
}

PackageSocket::~PackageSocket() {
	delete[] read_buff;
	delete[] write_buff;
	close(sock);
}

bool PackageSocket::canRead() {
	return !eot;
}

bool PackageSocket::read(char * b, size_t & len) {
	size_t b_idx=0;
	while(b_idx < len) {
		if(read_buff_used == read_buff_size) {
			ssize_t x = recv(sock,read_buff,max_buff_size, 0);
			if(x == -1) THROW_PE("recv failed\n");
			if(x == 0) {eot=true; break;}
			read_buff_used=0;
			read_buff_size=x;
		}
		while(b_idx < len && 
			  read_buff_used < read_buff_size) {
			if(read_escape) {
				b[b_idx] = read_buff[read_buff_used] == 'n'?'\n':read_buff[read_buff_used];
				b_idx++;
				read_escape=false;
			} else {
				if(read_buff[read_buff_used] == '\n') {
					read_buff_used++;
					len=b_idx;
					return true;
				} else if(read_buff[read_buff_used] == '\\') 
					read_escape=true;
				else {
					b[b_idx] = read_buff[read_buff_used];
					b_idx++;
				}
			}
			read_buff_used++;
		}
	}
	len = b_idx;
	return false;
}

std::string PackageSocket::readString(int maxlen) {
	char buff[maxlen+1];
	size_t x=1023;
	while(!read(buff,x)) {};
	buff[x] = '\0';
	return buff;
}

void PackageSocket::write(const char *b, size_t len, bool end_of_package) {
	if(len == 0) len=strlen(b);
	for(size_t i=0; i < len; ++i) {
		if(write_buff_used+2 >= max_buff_size) {
			size_t x=0;
			while(x < write_buff_used) {
				ssize_t s = send(sock, write_buff, write_buff_used-x, 0);
				if(s == -1) THROW_PE("send failed\n");
				x+=s;
			}
			write_buff_used=0;
		}
		if(b[i] == '\n' || b[i] == '\\') {
			write_buff[write_buff_used] = '\\';
			write_buff_used++;
			write_buff[write_buff_used] = (b[i] == '\n')?'n':'\\';
		}  else 
			write_buff[write_buff_used] = b[i];
		write_buff_used++;
	}
	if(end_of_package) {
		write_buff[write_buff_used] = '\n';
		write_buff_used++;
		size_t x=0;
		while(x < write_buff_used) {
			ssize_t s = send(sock, write_buff, write_buff_used-x, 0);
			if(s == -1) THROW_PE("send failed\n");
			x+=s;
		}
		write_buff_used=0;
	}
}
