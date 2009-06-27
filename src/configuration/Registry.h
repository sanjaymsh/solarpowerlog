/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (Registry), the license terms are:

 You can redistribute it and/or  modify it under the terms of the GNU Lesser
 General Public License (LGPL) as published by the Free Software Foundation;
 either version 3 of the License, or (at your option) any later version.

 This programm is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/**
 * \file Registry.h
 *
 * \page The class Registry: Configuration, Scheduler and Object Database
 *
 *  \date Created on: May 9, 2009
 *  \author Tobias Frost (coldtobi)
 *
 * \section LibConfig The Configuration Concept
 *
 * \sa \ref Registry
 *
 * Solarpowerlog uses libconfig as configuration backend.
 *
 * On startup it loads a file and extracts the configuration for the individual
 * components.
 *
 * The settings are structured to the indivdual sections, like general settings,
 * settings for inverters, settings for the loggers.
 *
 * The programm parses the basic layout of configuration.
 *
 * Out of the infos stored in configuration, the program generates a number of
 * objects.
 *
 * The generated object will be told its own configuration path (in libconfig
 * notation) to allow it to extract its own configuration. (This makes us the
 * the live easier, as we do not need to care which configuration items a object
 * requires: It knows best.)
 *
 * \section LibConfigRetrieving Retrieving Configurations
 *
 * As stated, solarpowerlog will instanciate objects with the informations found
 * in the configuration file.
 *
 * When creating the objects, the objects will also passes informations how
 * to access their informations. This is the configuration-path. For example
 * it may be "inverter.inverters.[0]", (but the object does not need to
 * understand what the meaning of the string is)
 *
 * This path can be directly used to retrieve the configuration. Also, this path
 * is validated (the object needs not to check for validity)
 *
 * A object should not access configuration elements prior checking for existance
 * and type correctness. Else be prepared to handle exceptions thrown by
 * libconfig.
 *
 * \note the object classes which can be configured have a function,
 * CheckConfig() called after object generation. The programm will terminate
 * if the configuration check fails.
 * If accessing the config during constructiion, you have to handle this
 * manually (or call checkconfig yourself).
 *
 * Here are some snippets which shows how to access the configurations:
 *
 * Sample "Check Config" snippet:
 * \code
 *
 *	// get settings object.
 *  	libconfig::Setting & set = Registry::Instance().GetSettingsForObject
 *  		(configurationpath);
 *
 *	// we want the setting "datsource"
 *	setting = "datasource";
 *	if (!set.exists(setting) || !set.getType()
 *		== libconfig::Setting::TypeString) {
 *		cerr << "Setting " << setting << " in " << configurationpath
 *			<< "." << name
 *			<< " missing or of wrong type (wanted a string)"
 *			<< endl;
 *		ret = false;
 *	} else {
 *		// setting exists and is of proper type
 *	}
 * \endcode
 *
 * Sample "Retrieve Configuration" snippet
 * \code
 * 	// note, this sample assunmes, that its config has been already checked
 * 	// for existance and type correctness (CheckConfig())
 * 	// get config object.
 *	libconfig::Setting & set = Registry::Instance().GetSettingsForObject(
 *		ConfigurationPath);
 *
 *	// local storage for the data
 *	string strhost, port;
 *	long timeout;
 *
 *	// looking up and storing
 *	set.lookupValue("tcpadr", strhost);
 *	set.lookupValue("tcpport", port);
 *
 *	// this shows how to handle an "optional" value: Check for existance
 *	// and if not, set a default value
 *	if (!set.lookupValue("tcptimeout", timeout))
 *		timeout = 3000;
 * \endcode
 *
 * \note it is planned to make it possible to "reload" configuration during
 * during runtime. But this will require some refactoring existing code,
 * so currently it is legal to cache configuration values.
 *
 * \sa Registry
 *
 * TODO enhance description.
 *
 * \section Registry The Object Register
 *
 * \sa Registry
 *
 * The class Registry also maintains a list of all objects it has created during
 * startup. At the moment these are Inverters and Loggers.
 *
 * This allows other components to search for other components by its
 * name.
 *
 * Solarpowerlog will make sure, that no objects will exist with identical name.
 *
 * \section TWS The Work Scheduler
 *
 * \sa CWorkScheduler
 *
 * The Registry also maintains one CWorkScheduler object which is can be used
 * by all objects.
 *
 * \note By sharing one scheduler for all objects, the objects does not need to
 * care about concurrency, as all the work is done sequencially. This is also
 * true for timed work.
 *
 * \subsection TWS_snippet
 *
 * This example code shows how to schedule work using the main scheduler:
 * \code
 * 		// Schedule the initialization and subscriptions later...
 *		ICommand *cmd = new ICommand(CMD_INIT, this, 0);
 *		Registry::GetMainScheduler()->ScheduleWork(cmd);
 * \endcode
 *
 * This example shows the usage with Timed Work. (1 second)
 * \code
 *		timespec ts = { 1, 0 };
 *		ICommand *cmd = new ICommand(CMD_INIT, this, 0);
 *		Registry::GetMainScheduler()->ScheduleWork(cmd, ts);
 * \endcode
 */

#ifndef REGISTRY_H_
#define REGISTRY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <list>
#include <utility>
#include <string>
#include <libconfig.h++>

#include <boost/noncopyable.hpp>

class CWorkScheduler;
class IInverterBase;

using namespace std;

/** The Registry stores some informations needed by several programm parts.
 * It also provides a global WorkScheduler.
 *
 * The Registry is a Singleton.
 *
 * \sa \ref LibConfig "The Configuration Concept"
 * \sa \ref Registry "The Object Register"
 * */
class Registry : boost::noncopyable
{
public:

	/** return the static, global object of the registry. */
	static Registry& Instance()
	{
		static Registry Instance;
		return Instance;
	}

	// C O N F I G U R A T I O N

	/** Shortcut to get the configuration.
	 *
	 * Please note, that it must be config_loaded beforehand.
	 * (but if coding for solarpowerlog, this will be done early
	 * in the startup phase, so one can expect a valid object here. */
	static libconfig::Config* Configuration()
	{
		return Registry::Instance().Config;
	}

	 /** (re)load configuration file
	 *
	 * Just brings it in memory... The old one will be deleted.
	 *
	 * \param [in] name Filename to load
	 *
	 * \returns false on error, true on success.
	 */
	bool LoadConfig( std::string name );

	/** Extract the settings-subset for a specific object,
	 * identified by section (like inverters) and name (like inverter_solarmax_1)
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
	 * and  GetSettingsForObject("inverters", "Inverter_1") will return the Settings object
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
	 * \warning This function is not intended to check if a setting exists!
	 */

	/** lookup a specific setting.
	 *
	 * \param section Configurationpath of the object.
	 * \param objname Additional path
	 *
	 * \warning libconfig might throw exceptions if the setting is not
	 * found.
	 * Howeverm, if using the configurationpath (supplied by the bootstrap
	 * code) and having objname empty, the path has been validated. */

	libconfig::Setting & GetSettingsForObject( std::string section,
		std::string objname = "" );

	/** search for an inverter/logger object with the object name
	 *
	 * \param name of the object
	 *
	 * \returns Pointer to the object or NULL if not found.
	*/
	IInverterBase* GetInverter( const string& name ) const;

	/** Add the a new created inveter/logger to the object database
	 *
	 * \note: This function is only for the bootstrap process!*/
	void AddInverter( const IInverterBase *inverter );

	/** Get the main scheduler to scheule work...
	 *
	 * \returns pointer to the scheduler */
	static CWorkScheduler *GetMainScheduler( void )
	{
		return Registry::Instance().mainscheduler;
	}

private:
	CWorkScheduler *mainscheduler;

protected:
	/** singleton: no object generation */
	Registry();
	virtual ~Registry();

private:
	libconfig::Config *Config;
	bool config_loaded;

	// TODO generalize this interface, as we might also store
	// other types of objects here.
	list<IInverterBase*> inverters;

};

#endif /* REGISTRY_H_ */
