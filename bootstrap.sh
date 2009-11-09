#!/bin/sh
# generate everything for the autotools...
rm aclocal.m4;
autoheader -B src &&  aclocal -I m4 && autoconf && automake --add-missing  
