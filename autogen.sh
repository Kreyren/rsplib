#!/bin/bash -e

if [ -e /usr/bin/dpkg-buildflags ] ; then
   export CPPFLAGS=`dpkg-buildflags --get CPPFLAGS`
   export CFLAGS=`dpkg-buildflags --get CFLAGS`
   export CXXFLAGS=`dpkg-buildflags --get CXXFLAGS`
   export LDFLAGS=`dpkg-buildflags --get LDFLAGS`
fi

./bootstrap && \
./configure --enable-kernel-sctp --enable-static --disable-shared --enable-qt --enable-csp --disable-hsmgtverify --enable-test-programs $@ && \
( gmake || make )
