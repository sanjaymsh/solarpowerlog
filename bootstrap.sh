#!/bin/sh
autoreconf --force --install -i --warnings=all
exit 0

# generate everything for the autotools...

# old version, kept for the moment. 
rm -rf aclocal.m4 >/dev/null 2>&1

mkdir config >/dev/null 2>&1
libtoolize && autoheader -B src &&  aclocal -I m4 && autoconf && automake --add-missing  

# generate for the extlibs under autoconf's regime
rm -rf extlibs/dbixx/.svn/ extlibs/dbixx/autom4te.cache/ extlibs/dbixx/config.guess extlibs/dbixx/config.sub  extlibs/dbixx/depcomp extlibs/dbixx/install-sh  extlibs/dbixx/ltmain.sh extlibs/dbixx/missing
[ -e extlibs/dbixx ]  && (cd extlibs/dbixx && ./autogen.sh)  || true
