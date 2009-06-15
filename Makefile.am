## Makefile.am -- Process this file with automake to produce Makefile.in
ACLOCAL_AMFLAGS = -I m4

AM_CXXFLAGS = $(BOOST_CPPFLAGS)

bin_PROGRAMS = solarpowerlog

solarpowerlog_SOURCES = src/interfaces/CCapability.cpp src/interfaces/CMutexHelper.cpp src/interfaces/factories/InverterFactoryFactory.cpp src/interfaces/factories/IInverterFactory.cpp src/interfaces/CWorkScheduler.cpp src/interfaces/CTimedWork.cpp src/interfaces/IConnect.cpp src/DataFilters/interfaces/factories/IDataFilterFactory.cpp src/DataFilters/interfaces/IDataFilter.cpp src/DataFilters/CDumpOutputFilter.cpp src/configuration/Registry.cpp src/Connections/CConnectDummy.cpp src/Connections/factories/IConnectFactory.cpp src/Connections/CConnectTCPAsio.cpp src/solarpowerlog.cpp src/Inverters/SputnikEngineering/CInverterFactorySputnik.cpp src/Inverters/SputnikEngineering/CInverterSputnikSSeries.cpp src/Inverters/interfaces/CNestedCapaIterator.cpp src/Inverters/interfaces/ICapaIterator.cpp src/Inverters/interfaces/InverterBase.cpp src/patterns/ICommandTarget.cpp src/patterns/IValue.cpp src/patterns/ICommand.cpp src/patterns/IObserverObserver.cpp src/patterns/IObserverSubject.cpp

solarpowerlog_CPPFLAGS = $(CONFIG_CFLAGS)
solarpowerlog_LDFLAGS = $(CONFIG_LIBS) $(BOOST_LDFLAGS) $(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_ASIO_LIB)

INCLUDES = -I@top_srcdir@/src 

LIBS = $(DEPS_LIBS)
