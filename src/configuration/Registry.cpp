/* ----------------------------------------------------------------------------
   solarpowerlog
   Copyright (C) 2009  Tobias Frost

   This file is part of solarpowerlog.

   Solarpowerlog is free software; However, it is dual-licenced
   as described in the file "COPYING".

   For this file (Registry.cpp), the license terms are:

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

/*
 * Registry.cpp
 *
 *  Created on: May 9, 2009
 *      Author: tobi
 *
 *
 */

#include "Registry.h"

#include <iostream>
#include "CWorkScheduler.h"

#include "Inverters/interfaces/InverterBase.h"

using namespace std;

/** constructor for the registry. */
Registry::Registry()
{
	loaded = false;
	Config = NULL;

}

/** (re)load configuration file
 *
 * Just brings it in memory... The old one will be deleted.
 *
 * \param [in] name Filename to load
 *
 * \returns false on error, true on success.
*/
bool Registry::LoadConfig(std::string name)
{
	if (Config) delete Config;
	loaded = false;
	Config = new libconfig::Config;
	try {
		Config->readFile(name.c_str());
	}
	catch (libconfig::ParseException ex)
	{
		std::cerr << "Error parsing configuration file " << name << " at Line "
			<< ex.getLine() << ". ("<< ex.getError() << ")"<<std::endl;
		delete Config;
		return false;
	}
	catch (libconfig::FileIOException ex)
	{
		std::cerr << "Error parsing configuration file " << name << ". IO Exception " << std::endl;
		delete Config;
		return false;
	}

	// Be more sloppy on datatypes -> automatically convert if possible.
	Config->setAutoConvert(true);
	loaded = true;

	return true;
}

/** Extract the settings-subset for a specific object, identified by section (like inverters) and name (like inverter_solarmax_1)
 *
 * ex:
 *
 * inverters = (
 * 					{ name = "Inverter_1";
 * 					  type = "Solarmax_XYZ";
 * 					  driver = "Sputnik_TCP";
 * 					  # (...)
 * 					},
 * 					{ name = "Inverter_2";
 * 					  type = "Solarmax_XYZ";
 * 					  driver = "Sputnik_TCP";
 * 					  # (...)
 * 					}
 * 				);
 *
 * and  GetSettingsForObject("inverters", "Invertert_1") will return the Settings object
 * for the group "inverters.[0]".
 *
 * Please note, that libconfig throws some exceptions: Especially, if the section is not found.
 * This is not handled here, as the Factories should check if the configuration is complete on
 * constructing them. (They also can add their own settings (default values)...
 *
 * [code]
 *
 *	libconfig::Setting &set = Registry::Instance().GetSettingsForObject("inverters", "Inverter_1");
 *	libconfig::Setting &new = set.add("NewPropertyNotSet",libconfig::Setting::TypeString);
 *	new = "New Default Setting";
 *
 * [/code]
 *
 * Snippet to retrieve Settings:
 * [code]
 * 		libconfig::Setting &set = Registry::Instance().GetSettingsForObject("inverters", "Inverter_1");
 * [/code]
 *
 */
libconfig::Setting & Registry::GetSettingsForObject(std::string section, std::string objname)
{

	libconfig::Setting &s = Config->lookup(section);

	if(objname == "" ) return s;

	for ( int i = 0 ; i < s.getLength() ; i ++ ) {

		std::string tmp =  s[i]["name"];
		if ( tmp  == objname ) return s[i];
	}

	// note: we cannot deliver a object here ... we simply do not have one!
	// As libconfig::SettingNotFoundException is private only, we also cannot
	// throw here. So, as convention, we return the root of the configuration here....
	// We "BUG" here, as it is the responsibility of the caller to ensure the
	// objects existence.
	// (Only Objects with a valid name should query their configuration)
	std::cerr<<"BUG: " << __FILE__ << ":" << __LINE__
		<< " --> Queried for unknown Object " << objname << " in section "
		<< section << std::endl;

	return Config->getRoot();
}

IInverterBase *Registry::GetInverter(const string & name) const
{

	list<IInverterBase*>::const_iterator iter;
	for ( iter = inverters.begin(); iter != inverters.end(); iter++)	{
		if ( (*iter)->GetName() == name ) return (*iter);
	}

	return 0;
}

void Registry::AddInverter(const IInverterBase *inverter)
{
	inverters.push_back((IInverterBase*)inverter);
}

/** destructor */
Registry::~Registry()
{
	if (Config) delete Config; Config = NULL;
	if (mainscheduler) delete mainscheduler;
}
