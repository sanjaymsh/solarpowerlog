## Makefile.am -- Process this file with automake to produce Makefile.in
ACLOCAL_AMFLAGS = -I m4

AM_CXXFLAGS = $(BOOST_CPPFLAGS)

#CTEMPLATE_LIB =-lctemplate
#CTEMPLATE_CFLAGS = -I ../extlibs

bin_PROGRAMS = solarpowerlog

solarpowerlog_SOURCES = interfaces/CCapability.cpp interfaces/CMutexHelper.cpp \
Inverters/factories/InverterFactoryFactory.cpp \
Inverters/factories/IInverterFactory.cpp interfaces/CWorkScheduler.cpp \
interfaces/CTimedWork.cpp Connections/interfaces/IConnect.cpp \
DataFilters/interfaces/factories/IDataFilterFactory.cpp \
DataFilters/interfaces/IDataFilter.cpp DataFilters/CDumpOutputFilter.cpp \
DataFilters/CCSVOutputFilter.cpp DataFilters/CHTMLWriter.cpp \
configuration/Registry.cpp configuration/ILogger.cpp \
Connections/CAsyncCommand.cpp Connections/CConnectDummy.cpp \
Connections/factories/IConnectFactory.cpp Connections/CConnectTCPAsio.cpp \
solarpowerlog.cpp Inverters/SputnikEngineering/CInverterFactorySputnik.cpp \
Inverters/SputnikEngineering/CInverterSputnikSSeries.cpp \
Inverters/interfaces/CNestedCapaIterator.cpp \
Inverters/interfaces/ICapaIterator.cpp Inverters/interfaces/InverterBase.cpp \
patterns/ICommandTarget.cpp patterns/IValue.cpp patterns/ICommand.cpp \
patterns/IObserverObserver.cpp patterns/IObserverSubject.cpp \
configuration/CConfigHelper.cpp 

solarpowerlog_CPPFLAGS = $(CONFIG_CFLAGS) $(LOG4CXX_CFLAGS) $(APR_CFLAGS) \
	$(APRUTIL_CFLAGS) $(CTEMPLATE_CFLAGS)

solarpowerlog_LDADD = $(CONFIG_LIBS) $(LOG4CXX_LIBS) $(APR_LIBS) \
	$(APRUTIL_LIBS) $(BOOST_LDFLAGS) $(BOOST_THREAD_LIB) \
	$(BOOST_DATE_TIME_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_ASIO_LIB) \
	$(BOOST_PROGRAM_OPTIONS_LIB) $(WIN32_LIBS) $(CTEMPLATE_LIB)

LIBS = $(DEPS_LIBS)  

# add the search path to the linker path 
solarpowerlog_LDFLAGS = $(pwd) -L ../extlibs/ctemplate 

# rule to make the ctemplate library 
libctemplate.a: 
	(cd ../extlibs/ctemplate/ && $(MAKE))  

# rule to clean up local stuff, like the ctemplate library 
clean-local: libctemplateclean

libctemplateclean:
	(cd ../extlibs/ctemplate/ && $(MAKE) clean)  

# do not forget to make the template library 
all: ${CTEMPLATE_DEPS}