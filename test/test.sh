#!/bin/bash
GARGS=$@
function t() {
    name=$1
    r=$(cat ../common/results.hh | sed -nre "s/#define $2[ \t]*([0-9]+)/\1/p")
    shift
    shift
    rm -rf x
    x=$(../bin/client $GARGS $@ | tee x | head -n1)
    if [ "$r" == "$x" ]; then
	printf "%-30s [ OK ]\n" "$name"
    else
	printf "%-30s [ !! ]\n" "$name"
	cat x | sed -re 's/.*/    \0/'
	echo
    fi
    rm -rf x
}
function t2() {
    t "$1" "$2" judge test -i "$3"
}

rm -rf ../test.tar.bz2
cd add
tar -cjf ../test.tar.bz2 *
cd ..
t "dispose test" RUN_SUCCESS dispose test
t "import test" RUN_SUCCESS import test.tar.bz2
t2 "good test" RUN_SUCCESS good.cc
t2 "Precentation error" RUN_PRESENTATION_ERROR pres.cc
t2 "wrong output" RUN_WRONG_OUTPUT wrong.cc
t2 "hack" RUN_EXIT_NOT_ZERO hack.cc
t2 "sigsegv" RUN_INVALID_MEMORY_REFERENCE sigsegv.cc
t2 "Memory" RUN_MEMORY_LIMIT_EXCEEDED memory.cc
t2 "Compile error" RUN_COMPILATION_ERROR compile.cc
#t2 "Bussy wait" RUN_TIME_LIMIT_EXCEEDED bussywait.cc
#t2 "Sleep wait" RUN_TIME_LIMIT_EXCEEDED sleep.cc
#t2 "Fork bomb" RUN_TIME_LIMIT_EXCEEDED fork.cc
