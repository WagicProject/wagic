#!/bin/sh -ex

# we're building a PSP binary here
cd JGE
make -j 8
cd ..
cd projects/mtg
mkdir objs
make -j 8

# let's try an Intel linux binary
cd ../..
qmake projects/mtg/wagic-qt.pro CONFIG+=console CONFIG+=debug
make -j 8

# and finish by running the testsuite
./wagic
