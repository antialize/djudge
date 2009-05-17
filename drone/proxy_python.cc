//-*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
#include <unistd.h>
int main(int, char ** argv) {
	argv[0] = (char*)("python");
	execvp(argv[0], argv);
	return 1;
}
