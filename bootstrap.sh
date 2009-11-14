#!/bin/sh
# generate everything for the autotools...
rm -rf aclocal.m4 >/dev/null 2>&1
mkdir config >/dev/null 2>&1
autoheader -B src &&  aclocal -I m4 && autoconf && automake --add-missing  
