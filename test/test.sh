#!/bin/bash
GARGS=$@
function t() {
    name=$1
    r=$2
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
t "dispose test" 0 dispose test
t "import test" 0 import test.tar.bz2
t2 "good test" 0 good.cc
t2 "Precentation error" 2 pres.cc
t2 "wrong output" 14 wrong.cc
t2 "hack" 1 hack.cc
t2 "sigsegv" 5 sigsegv.cc
t2 "Memory" 5 memory.cc
t2 "Bussy wait" 3 bussywait.cc
t2 "Sleep wait" 3 sleep.cc
t2 "Fork bomb" 3 fork.cc
t2 "Compile error" 14 compile.cc