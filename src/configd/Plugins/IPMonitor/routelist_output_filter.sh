#!/bin/sh
sed -e 's/^\(Process:.*\[\)[0-9][0-9]*/\1XXXX/'			\
    -e 's/^\(Load Address:.*0x\)[0-9a-f][0-9a-f]*/\1XXXX/'	\
    -e 's/^\(Parent Process:.*\[\)[0-9][0-9]*/\1XXXX/'		\
    -e 's/^\(Date\/Time: *\)[0-9].*/\1XXXX/'			\
    -e '/Process [0-9][0-9]*: [0-9][0-9]* nodes malloced/d'	\
    -e 's/^\(Process \)[0-9][0-9]*:\(.*\)/\1XXXX\2/' 		\
    -e 's/^\(Path: \)\(.*$\)/\1XXXX/'				\
    -e 's/^\(OS Version: \)\(.*$\)/\1XXXX/'
