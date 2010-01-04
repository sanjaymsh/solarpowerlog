/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (solarpowerlog.cpp), the license terms are:

 You can redistribute it and/or modify it under the terms of the GNU
 General Public License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This programm is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/** \defgroup Concepts Developer: Basic Concepts
 *
 * This section describes some basics concepts used in deveoping the software.
 */

/**
 * \file solarpowerlog.cpp
 *
 * \mainpage
 * <h1> Welcome to solarpowerlog's developer documentation. </h1>
 *
 * This documentation purpose is to understand the internals and conecpts used,
 * so that the software can easily be enhanced.
 *
 * However, the best way to understand the programm is jumping into cold water:
 * Fire up your debugger and look how the programm executes.
 *
 * \sa \ref mainBasicConcepts "Basic Concepts"
 *
 * \page mainBasicConcepts Basic Concepts
 *
 * To get an overview how solarpowerlog is designed, lets take a look at some
 * basic concepts.
 *
 * \section mbcinterfaces "Code against interfaces, not implementations"
 *
 * All base classes are designed as interfaces. This allows loosly coupled
 * objects and also allows to easier code reuse.
 *
 * For example, the connection classes are defined through IConnect. When using
 * IConnects, the inverter simply does not need to know which is its
 * communication method, it will just use the interface and will be fine.
 *
 * \section mbcfactories Design Pattern: Factories
 *
 * Usually object generation is done by factories. The Factories gets an
 * identifier (usually a string) and return the created object.
 *
 * Factories allows that new specializations of interfaces can be added to
 * the programm without the need to change any of the other classes.
 *
 * For example, if you add a fancy bluetooth class, you just create a IConnect
 * based CConnectionBluetooth, implement it and add its id-string to the
 * IConnectFactory.
 * Now, all the inverters can instanciate (via their comms settings) a
 * CConnectionBluetooth without knowing actually knowing the details.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector>
#include <string>
#include <iostream>
#include <iterator>

#if defined HAVE_LIBLOG4CXX
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/xml/domconfigurator.h>
#endif

#include "configuration/ILogger.h"

#ifdef HAVE_CMDLINEOPTS
#include <boost/program_options.hpp>
#endif

#include "configuration/Registry.h"
#include "interfaces/CWorkScheduler.h"
#include "patterns/ICommand.h"
#include "patterns/ICommandTarget.h"

#include "Inverters/factories/IInverterFactory.h"
#include "Inverters/factories/InverterFactoryFactory.h"
#include "Inverters/interfaces/InverterBase.h"

#include "DataFilters/interfaces/factories/IDataFilterFactory.h"
#include "configuration/CConfigHelper.h"

using namespace std;

/** this array of string specifies which sections int the config file must be present.
 * The programm will abort if any of these is missing.
 */
static const char *required_sections[] = { "application", "inverter",
		"inverter.inverters", "logger", "logger.loggers" };

/** Just dump the read config to cout.... (without values, as for these one must know the type  forehand )

 Use with:
 [code]
 libconfig::Setting &set = Registry::Configuration()->getRoot(); if (set.getName()) cout << set.getName(); else cout << "anonymous";
 cout << " has the Path \"" << set.getPath() << "\" and Type " << set.getType() << " and has a Lenght of " <<  set.getLength() << endl;

 DumpSettings(set);
 [/code]
 */
void DumpSettings(libconfig::Setting &set) {

	if (set.getPath() != "")
		cout << set.getPath();

	if (!set.getName()) {
		cout << "(anonymous) ";
	}

	if (set.isAggregate()) {
		cout << "\t is aggregate";
	};
	if (set.isArray()) {
		cout << "\t is array ";
	}
	if (set.isGroup()) {
		cout << "\t is group ";
	}
	if (set.isList()) {
		cout << "\t is list ";
	}
	//if (set.getPath() != "" ) cout << set.getPath() << "." ;
	if (set.isNumber())
		cout << " \t is number ";
	if (set.isScalar())
		cout << " \t is scalar ";

	cout << endl;

	for (int i = 0; i < set.getLength(); i++) {

		libconfig::Setting &s2 = set[i];
		DumpSettings(s2);
	}
}

int main(int argc, char* argv[]) {
	bool error_detected = false;
	bool dumpconfig = false;
	string configfile = "solarpowerlog.conf";

#ifdef  HAVE_CMDLINEOPTS
	using namespace boost::program_options;

	options_description desc("Programm Options");
	desc.add_options()
		("help", "this message")
		("conf,c",
			value<string> (&configfile), "specify configuration file")
		("version,v", "display solarpowerlog version")
		("dumpcfg", value<bool> (&dumpconfig)->zero_tokens(), "Dump configuration structure, then exit")
	//	("foreground,f", value<int> (&debug_level),
	//		"do not daemonize, stay in foreground. (currently unused)")
	;

	variables_map vm;
	try {
		store(parse_command_line(argc, argv, desc), vm);
		notify(vm);
	} catch (exception e) {
		cerr << desc << "\n";
		return 1;

	}

	if (vm.count("help")) {
		cout << desc << "\n";
		return 0;
	}

	if (vm.count("version")) {
		cout << PACKAGE_STRING << endl;
		return 0;
	}

#else
	if (argc > 1) {
		(void) argv; // remove warning unused parameter.
		cerr << "This version does not support command line options." << endl;
		_exit(1);
	}
#endif

	/* Loading configuration file */
	if (!Registry::Instance().LoadConfig(configfile)) {
		cerr << "Could not load configuration " << configfile << endl;
		_exit(1);
	}

	if (dumpconfig) {
		DumpSettings(Registry::Configuration()->getRoot());
		_exit(0);
	}

	// Note: As a limitation of libconfig, one cannot create the configs
	// structure.
	// Therefore we check here for the basic required sections and abort,
	// if one is not existing.
	{
		libconfig::Config *cfg = Registry::Configuration();
		libconfig::Setting &rt = cfg->getRoot();

		for (unsigned int i = 0; i < sizeof(required_sections) / sizeof(char*); i++) {
			if (!rt.exists(required_sections[i])) {
				cerr << " Configuration Check: Required Section "
						<< required_sections[i] << " missing" << endl;
				error_detected = true;
			}
		}
	}

	if (error_detected) {
		cerr << "Error detected" << endl;
		_exit(1);
	}

#if defined HAVE_LIBLOG4CXX
	// Activate Logging framework
	{
		using namespace log4cxx;
		string tmp;
		LoggerPtr l = Logger::getRootLogger();

		CConfigHelper global("application");
		global.GetConfig("dbglevel", tmp, (std::string) "ERROR");
		l->setLevel(Level::toLevel(tmp));

		try {
			// Choose your poison .. aem .. config file format
			if (global.GetConfig("logconfig", tmp)) {
				if (tmp.substr(tmp.length() - 4, string::npos) == ".xml") {
					xml::DOMConfigurator::configure(tmp);
				} else {
					PropertyConfigurator::configure(tmp);
				}

			} else {
				BasicConfigurator::configure();
			}
		} catch (...) {
			cerr << "WARNING: Could not configure logging." << endl;
		}

		LOG4CXX_INFO(l,"Logging set up.");
	}
#endif

	ILogger mainlogger;

	/** bootstraping the system */
	LOG_DEBUG(mainlogger,
			"Instanciating Inverter objects");

	/** create the inverters via its factories. */
	{
		string section = "inverter.inverters";
		libconfig::Setting &rt = Registry::Configuration()->lookup(section);

		for (int i = 0; i < rt.getLength(); i++) {
			std::string name;
			std::string manufactor;
			std::string model;

			try {
				name = (const char *) rt[i]["name"];
				manufactor = (const char *) rt[i]["manufactor"];
				model = (const char *) rt[i]["model"];
				LOG_DEBUG(mainlogger,
						name << " " << manufactor );
			} catch (libconfig::SettingNotFoundException e) {
				LOG_FATAL(mainlogger,
						"Configuration Error: Required Setting was not found in \""
						<< e.getPath() << '\"');
				_exit(1);
			}

			if (Registry::Instance().GetInverter(name)) {
				LOG_FATAL(mainlogger, "Inverter " << name
						<< " declared more than once");
				_exit(1);
			}

			IInverterFactory *factory =
					InverterFactoryFactory::createInverterFactory(manufactor);
			if (!factory) {
				LOG_FATAL(mainlogger,
						"Unknown inverter manufactor \""
						<< manufactor << '\"');
				_exit(1);
			}

			IInverterBase *inverter = factory->Factory(model, name,
					rt[i].getPath());

			if (!inverter) {
				LOG_FATAL(mainlogger,
						"Cannot create Inverter model "
						<< model << "for manufactor \""
						<< manufactor << '\"');

				LOG_FATAL(mainlogger,
						"Supported models are: "
						<< factory->GetSupportedModels());
				_exit(1);
			}

			if (!inverter->CheckConfig()) {
				LOG_FATAL(mainlogger,
						"Inverter " << name << " ( "
						<< manufactor << ", " << model
						<< ") reported configuration error");
				_exit(1);
			}

			Registry::Instance().AddInverter(inverter);
			// destroy the (used) factory.
			delete factory;
		}
	}

	LOG_DEBUG(mainlogger, "Instanciating DataFilter objects");

	{
		IDataFilterFactory factory;
		/** create the data filters via its factories. */

		string section = "logger.loggers";
		libconfig::Setting & rt = Registry::Configuration()->lookup(section);

		for (int i = 0; i < rt.getLength(); i++) {
			std::string name;
			std::string previousfilter;
			std::string type;

			try {
				name = (const char *) rt[i]["name"];
				previousfilter = (const char *) rt[i]["datasource"];
				type = (const char *) rt[i]["type"];

				LOG_DEBUG(mainlogger,
						"Datafilter " << name << " ("
						<< type << ") connects to "
						<< previousfilter <<
						" with Config-path " << rt[i].getPath());

			} catch (libconfig::SettingNotFoundException e) {
				LOG_FATAL(mainlogger,
						"Configuration Error: Required Setting was not found in \""
						<< e.getPath() << '\"' );
				_exit(1);
			}

			// TODO Also check for duplicate DataFilters.
			if (Registry::Instance().GetInverter(name)) {
				LOG_FATAL(mainlogger,
						"CONFIG ERROR: Inverter or Logger Nameclash: "
						<< name << " declared more than once"
				);
				_exit(1);
			}

			IDataFilter *filter = factory.Factory(rt[i].getPath());

			if (!filter) {
				LOG_FATAL(mainlogger,
						"Couldn't create DataFilter " << name
						<< "(" << type << ") connecting to "
						<< previousfilter << " Config-path " << rt[i].getPath()
				);
				_exit(1);
			}

			if (!filter->CheckConfig()) {
				LOG_FATAL(mainlogger,
						"DataFilter " << name << "(" << type
						<< ") reported config error"
						<< previousfilter );
				_exit(1);
			}

			Registry::Instance().AddInverter(filter);

			// Filter is ready.
		}
	}

	LOG_DEBUG(mainlogger, "Entering main loop");

	while (true) {
		// cerr << "." << flush;
		Registry::GetMainScheduler()->DoWork(true);
		// cerr << ":" << flush;
	}

	return 0;
}
