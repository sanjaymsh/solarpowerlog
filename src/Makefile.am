## Makefile.am -- Process this file with automake to produce Makefile.in
ACLOCAL_AMFLAGS = -I m4

AM_CXXFLAGS = $(BOOST_CPPFLAGS)

bin_PROGRAMS = solarpowerlog

solarpowerlog_SOURCES = interfaces/CCapability.cpp interfaces/CMutexHelper.cpp \
interfaces/factories/InverterFactoryFactory.cpp \
interfaces/factories/IInverterFactory.cpp interfaces/CWorkScheduler.cpp \
interfaces/CTimedWork.cpp interfaces/IConnect.cpp \
DataFilters/interfaces/factories/IDataFilterFactory.cpp \
DataFilters/interfaces/IDataFilter.cpp DataFilters/CDumpOutputFilter.cpp \
DataFilters/CCSVOutputFilter.cpp \
configuration/Registry.cpp Connections/CConnectDummy.cpp \
Connections/factories/IConnectFactory.cpp Connections/CConnectTCPAsio.cpp \
solarpowerlog.cpp Inverters/SputnikEngineering/CInverterFactorySputnik.cpp \
Inverters/SputnikEngineering/CInverterSputnikSSeries.cpp \
Inverters/interfaces/CNestedCapaIterator.cpp \
Inverters/interfaces/ICapaIterator.cpp Inverters/interfaces/InverterBase.cpp \
patterns/ICommandTarget.cpp patterns/IValue.cpp patterns/ICommand.cpp \
patterns/IObserverObserver.cpp patterns/IObserverSubject.cpp \
configuration/CConfigHelper.cpp 

solarpowerlog_CPPFLAGS = $(CONFIG_CFLAGS)
solarpowerlog_LDADD = $(CONFIG_LIBS) $(BOOST_LDFLAGS) $(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_ASIO_LIB) $(BOOST_PROGRAM_OPTIONS_LIB) $(WIN32_LIBS)

LIBS = $(DEPS_LIBS)
