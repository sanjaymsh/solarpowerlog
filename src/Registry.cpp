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

Registry::Registry() {
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

Registry::~Registry() {
	// TODO Auto-generated destructor stub
}
