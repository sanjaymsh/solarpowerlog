## Makefile.am -- Process this file with automake to produce Makefile.in
ACLOCAL_AMFLAGS = -I m4

if MAKE_DBIXX 
	DBIXX_SUBDIRS=extlibs/dbixx
endif

SUBDIRS=$(DBIXX_SUBDIRS) src

# install some extras into the dist files...
dist_doc_DATA = AUTHORS NEWS README ChangeLog example_confs/solarpowerlog.conf
