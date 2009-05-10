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
 * This is not handled here, as the Factories should check if the configuratoin is complete on
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

/** destructor */
Registry::~Registry()
{
	// TODO Auto-generated destructor stub
}
