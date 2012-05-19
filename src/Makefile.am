## Makefile.am -- Process this file with automake to produce Makefile.in
ACLOCAL_AMFLAGS = -I m4

AM_CXXFLAGS = $(BOOST_CPPFLAGS)


#if MAKE_DBIXX 
#        DBIXX_LIB =-ldbixx
#	DBIXX_LIB_PATH=-L../extlibs/dbixx
#else
	DBIXX_LIB =
#endif

#DBIXX_LIB =-ldbixx

bin_PROGRAMS = solarpowerlog

solarpowerlog_SOURCES = interfaces/CCapability.cpp interfaces/CMutexHelper.cpp \
Inverters/factories/InverterFactoryFactory.cpp \
Inverters/factories/IInverterFactory.cpp interfaces/CWorkScheduler.cpp \
interfaces/CTimedWork.cpp Connections/interfaces/IConnect.cpp \
DataFilters/interfaces/factories/IDataFilterFactory.cpp \
DataFilters/HTMLWriter/formatter/IFormater.cpp \
DataFilters/HTMLWriter/formatter/CFormaterWebRootStrip.cpp \
DataFilters/HTMLWriter/formatter/CFormatterSearchCSVEntry.cpp \
DataFilters/interfaces/IDataFilter.cpp DataFilters/CDumpOutputFilter.cpp \
DataFilters/CCSVOutputFilter.cpp DataFilters/HTMLWriter/CHTMLWriter.cpp \
configuration/Registry.cpp configuration/ILogger.cpp \
Connections/CAsyncCommand.cpp Connections/CConnectDummy.cpp \
Connections/CSharedConnection.cpp Connections/CSharedConnectionMaster.cpp \
Connections/CSharedConnectionSlave.cpp Connections/CConnectSerialAsio.cpp \
Connections/factories/IConnectFactory.cpp Connections/CConnectTCPAsio.cpp \
solarpowerlog.cpp Inverters/SputnikEngineering/CInverterFactorySputnik.cpp \
Inverters/SputnikEngineering/CInverterSputnikSSeries.cpp \
Inverters/SputnikEngineering/CSputnikCommand.cpp \
Inverters/SputnikEngineering/ISputnikCommand.cpp \
Inverters/SputnikEngineering/CSputnikCommandSoftwareVersion.cpp \
Inverters/DummyInverter/CInverterDummy.cpp  \
Inverters/DummyInverter/CInverterFactoryDummy.cpp \
Inverters/interfaces/CNestedCapaIterator.cpp \
Inverters/interfaces/ICapaIterator.cpp Inverters/interfaces/InverterBase.cpp \
patterns/ICommandTarget.cpp patterns/IValue.cpp patterns/ICommand.cpp \
patterns/IObserverObserver.cpp patterns/IObserverSubject.cpp \
configuration/CConfigHelper.cpp ctemplate/ctemplate.c \
interfaces/CDebugHelper.cpp \
Inverters/BasicCommands.h \
interfaces/CCapability.h interfaces/CMutexHelper.h \
Inverters/factories/InverterFactoryFactory.h \
Inverters/factories/IInverterFactory.h interfaces/CWorkScheduler.h \
interfaces/CTimedWork.h Connections/interfaces/IConnect.h \
DataFilters/interfaces/factories/IDataFilterFactory.h \
DataFilters/HTMLWriter/formatter/IFormater.h \
DataFilters/HTMLWriter/formatter/CFormaterWebRootStrip.h \
DataFilters/HTMLWriter/formatter/CFormatterSearchCSVEntry.h \
DataFilters/interfaces/IDataFilter.h DataFilters/CDumpOutputFilter.h \
DataFilters/CCSVOutputFilter.h DataFilters/HTMLWriter/CHTMLWriter.h \
configuration/Registry.h configuration/ILogger.h \
Connections/CAsyncCommand.h Connections/CConnectDummy.h \
Connections/CSharedConnection.h Connections/CSharedConnectionMaster.h \
Connections/CSharedConnectionSlave.h Connections/CConnectSerialAsio.h \
Connections/factories/IConnectFactory.h Connections/CConnectTCPAsio.h \
Inverters/SputnikEngineering/CInverterFactorySputnik.h \
Inverters/SputnikEngineering/CInverterSputnikSSeries.h \
Inverters/SputnikEngineering/CSputnikCommand.h \
Inverters/SputnikEngineering/ISputnikCommand.h \
Inverters/SputnikEngineering/CSputnikCommandSoftwareVersion.h \
Inverters/DummyInverter/CInverterDummy.h  \
Inverters/DummyInverter/CInverterFactoryDummy.h \
Inverters/interfaces/CNestedCapaIterator.h \
Inverters/interfaces/ICapaIterator.h Inverters/interfaces/InverterBase.h \
patterns/ICommandTarget.h patterns/IValue.h patterns/ICommand.h \
patterns/IObserverObserver.h patterns/IObserverSubject.h \
configuration/CConfigHelper.h ctemplate/ctemplate.h porting.h \
Inverters/Capabilites.h patterns/CValue.h interfaces/CDebugHelper.h

solarpowerlog_CPPFLAGS = $(CONFIG_CFLAGS) $(LOG4CXX_CFLAGS) $(APR_CFLAGS) \
	$(APRUTIL_CFLAGS) $(CTEMPLATE_CFLAGS)

solarpowerlog_LDADD = $(CONFIG_LIBS) $(LOG4CXX_LIBS) $(APR_LIBS) \
	$(APRUTIL_LIBS) $(BOOST_LDFLAGS) $(BOOST_THREAD_LIBS) \
	$(BOOST_DATE_TIME_LIBS) $(BOOST_SYSTEM_LIBS) $(BOOST_ASIO_LIBS) \
	$(BOOST_PROGRAM_OPTIONS_LIBS) $(WIN32_LIBS) 

LIBS = $(DEPS_LIBS)  

# add the search path to the linker path 
solarpowerlog_LDFLAGS = $(pwd) $(DBIXX_LIB_PATH)

# rule to make the ctemplate library 
#libctemplate.a: 
#	(cd ../extlibs/ctemplate/ && $(MAKE))  

# rule to clean up local stuff, like the ctemplate library 
#clean-local: libctemplateclean
#
#libctemplateclean:
#	(cd ../extlibs/ctemplate/ && $(MAKE) clean)  
#
# do not forget to make the template library 
#all: ${CTEMPLATE_DEPS}
