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

#include <vector>
#include <string>
#include <iostream>
#include "configuration/Registry.h"
#include "interfaces/CWorkScheduler.h"
#include "patterns/ICommand.h"
#include "patterns/ICommandTarget.h"
#include "interfaces/CTimedWork.h"
#include "interfaces/factories/IInverterFactory.h"
#include "interfaces/factories/InverterFactoryFactory.h"
#include "Inverters/interfaces/InverterBase.h"

#include <cc++/socket.h>
#include <cc++/address.h>
#include "Connections/CConnectTCP.h"
#include "Inverters/SputnikEngineering/CInverterSputnikSSeries.h"
#include "DataFilters/interfaces/factories/IDataFilterFactory.h"

#include <asio.hpp>

#include <iostream>

using namespace std;

// Debug Code dumpster....


/** Just dump the read config to cout.... (without values, as for these one must know the type  forehand )

 Use with:
 [code]
 libconfig::Setting &set = Registry::Configuration()->getRoot(); if (set.getName()) cout << set.getName(); else cout << "anonymous";
 cout << " has the Path \"" << set.getPath() << "\" and Type " << set.getType() << " and has a Lenght of " <<  set.getLength() << endl;

 DumpSettings(set);
 [/code]
 */
void DumpSettings( libconfig::Setting &set )
{

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

static const char *required_sections[] = { "application", "inverter",
	"inverter.inverters", "logger" };

using namespace std;
using namespace asio;

int main()
{
#if 0
	// getting asio known...
	asio::io_service my_io_service;
	asio::error_code ec,ec2;

	asio::ip::tcp::resolver resolver(my_io_service);
	asio::ip::tcp::socket socket(my_io_service);
	asio::ip::tcp::resolver::query query("127.0.0.1", 12345);
	asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
	asio::ip::tcp::resolver::iterator end; // End marker.
	while (iter != end) {
		asio::ip::tcp::endpoint endpoint = *iter++;
		std::cout << endpoint << std::endl;
		ec2 = socket.connect(endpoint, ec);
		if (ec) cerr << "ERROR "<< ec;
	}



	return 0;
#endif

#if 0
	using boost::asio::ip::tcp;

	boost::asio::io_service io_service;
	tcp::resolver resolver(io_service);
	tcp::resolver::query query("localhost", "12345");
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	tcp::resolver::iterator end;

	tcp::socket socket(io_service);
	boost::system::error_code error = boost::asio::error::host_not_found;
	while (error && endpoint_iterator != end) {
		socket.close();
		socket.connect(*endpoint_iterator++, error);
	}
	if (error)
	throw boost::system::system_error(error);

	socket.write_some("Test");

	return 0;
#endif

	bool error_detected = false;

	/** Loading configuration file */
	cout << "Generating Registry" << endl;
	// TODO avoid hardcoded filename. Get it from parameters.
	if (!Registry::Instance().LoadConfig("solarpowerlog.conf")) {
		cerr << "Could not load configuration " << endl;
		_exit(1);
	}

	// Note: As a limitation of libconfig, one cannot create the configs
	// structure.
	// Therefore we check here for the basic required sections and abort,
	// if one is not existing.
	{
		libconfig::Config *cfg = Registry::Configuration();
		libconfig::Setting &rt = cfg->getRoot();

		for (unsigned int i = 0; i < sizeof(required_sections)
			/ sizeof(char*); i++) {
			if (!rt.exists(required_sections[i])) {
				cerr
					<< " Configuration Check: Required Section "
					<< required_sections[i] << " missing"
					<< endl;
				error_detected = true;
			}
		}
	}

	if (error_detected) {
		cerr << "Error detected" << endl;
		_exit(1);
	}

	/** bootstraping the system */

	/** create the main scheduler. */
	Registry::Instance().setMainScheduler(new CWorkScheduler);

	/** create the inverters via its factories. */
	{
		string section = "inverter.inverters";
		libconfig::Setting &rt = Registry::Configuration()->lookup(
			section);

		// DumpSettings(rt);

		cout << rt.getLength() << endl;

		for (int i = 0; i < rt.getLength(); i++) {
			std::string name;
			std::string manufactor;
			std::string model;

			try {
				name = (const char *) rt[i]["name"];
				manufactor = (const char *) rt[i]["manufactor"];
				model = (const char *) rt[i]["model"];
				cout << name << " " << manufactor << endl;
			} catch (libconfig::SettingNotFoundException e) {
				cerr
					<< "Configuration Error: Required Setting was not found in \""
					<< e.getPath() << '\"' << endl;
				_exit(1);
			}

			if (Registry::Instance().GetInverter(name)) {
				cerr << "Inverter " << name
					<< " declared more than once" << endl;
				_exit(1);
			}

			IInverterFactory *factory =
				InverterFactoryFactory::createInverterFactory(
					manufactor);
			if (!factory) {
				cerr
					<< "Cannot create Inverter for manufactor \""
					<< manufactor << '\"';
				cerr
					<< " (Cannot create factory. Maybe mispelled manufactor?"
					<< endl;
				_exit(1);
			}

			cout << "path " << rt[i].getPath() << endl;

			IInverterBase *inverter = factory->Factory(model, name,
				rt[i].getPath());

			if (!inverter) {
				cerr << "Cannot create Inverter model "
					<< model << "for manufactor \""
					<< manufactor << '\"';
				cerr
					<< " (Cannot create factory. Maybe mispelled model?)"
					<< endl;
				cerr
					<< " The factory says, it supports the following models:"
					<< endl;
				cerr << factory->GetSupportedModels() << endl;
				_exit(1);
			}

			if (!inverter->CheckConfig()) {
				cerr << "Inverter " << name << " ( "
					<< manufactor << ", " << model
					<< ") reported configuration error";
				_exit(1);
			}

			Registry::Instance().AddInverter(inverter);
			// destroy the (used) factory.
			delete factory;
		}
	}

	{
		IDataFilterFactory factory;
		/** create the inverters via its factories. */

		string section = "logger.loggers";
		libconfig::Setting & rt = Registry::Configuration()->lookup(
			section);

		for (int i = 0; i < rt.getLength(); i++) {
			std::string name;
			std::string previousfilter;
			std::string type;

			try {
				name = (const char *) rt[i]["name"];
				previousfilter
					= (const char *) rt[i]["datasource"];
				type = (const char *) rt[i]["type"];

				cout << "DEBUG: Datafilter " << name << " ("
					<< type << ") connects to "
					<< previousfilter << endl;
				cout << "Config-path " << rt[i].getPath()
					<< endl;

			} catch (libconfig::SettingNotFoundException e) {
				cerr
					<< "Configuration Error: Required Setting was not found in \""
					<< e.getPath() << '\"' << endl;
				_exit(1);
			}

			// TODO Also check for duplicate DataFilters.
#if 0
			if (Registry::Instance().GetInverter(name)) {
				cerr << "Inverter " << name
				<< " declared more than once" << endl;
				_exit(1);
			}
#endif

			IDataFilter *filter = factory.Factory(type, name,
				rt[i].getPath());

			if (!filter) {
				cerr << "Couldn't create DataFilter " << name
					<< "(" << type << ") connecting to "
					<< previousfilter << endl;
				cout << "Config-path " << rt[i].getPath()
					<< endl;
				_exit(1);
			}

			if (!filter->CheckConfig()) {

				cerr << "DataFilter " << name << "(" << type
					<< ") reporting config error"
					<< previousfilter << endl;
				_exit(1);
			}

			// Filter is ready.

			// TODO Register to registry.
		}
	}

	while (true) {
		// cerr << "." << flush;
		Registry::GetMainScheduler()->DoWork(true);
		// cerr << ":" << flush;
	}

	return 0;
}
