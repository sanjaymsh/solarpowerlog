/*
 * Registry.h
 *
 *  Created on: May 9, 2009
 *      Author: tobi
 *
 * The Registry Class ist the central point to store configuration relevant for operations.
 *
 * Each Class requiring a configuration is free to use one of its objects.
 *
 */

#ifndef REGISTRY_H_
#define REGISTRY_H_


#include <utility>
#include <string>
#include <libconfig.h++>

using namespace std;


class Registry {
public:

	static Registry& Instance() {
		// TODO Make sure this is thread safe (might already be, depending on
		// the instanciation time: When does C++ instanciate static Objects?)
		static Registry Instance;
		return Instance;
	}

	/* Shortcut to get the configuration.
	 * Please note, that it must be loaded beforehand. */
	static libconfig::Config* Configuration()
	{
		return Registry::Instance().Config;
	}

	bool LoadConfig(std::string name);

	libconfig::Setting & GetSettingsForObject(std::string section, std::string objname);

protected:
	Registry();
	Registry (const Registry& other) {};
	virtual ~Registry();

private:
	libconfig::Config *Config;

	bool loaded;


};

#endif /* REGISTRY_H_ */
