/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CConfigHelper.h), the license terms are:

 You can redistribute it and/or  modify it under the terms of the GNU Lesser
 General Public License (LGPL) as published by the Free Software Foundation;
 either version 3 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/** \file CConfigHelper.h
 *
 *  \date Jul 4, 2009
 *  \author Tobias Frost (coldtobi)
 */

/** \page Config_new Handling the Configuration
 *
 * \section ConfigNew_Overview Overview
 *
 * Solarpowerlog uses a configuration file to get the settings of the individual
 * components. It even decided out of the config file which components are
 * needed ans wich objects needs to work together.
 *
 * For easier handling of the configuration, solarpowerlog provides methods to
 * gather the settings.
 *
 * First, every object gets a so-called configuaration path. This is a string,
 * describing where the configuration of the object is located. (As currently
 * libconfig is used, this is the "path" of libconfig.
 * Every object should safe this path.
 *
 * Second, a helper class, CConfigHelper exists. This class has some methods
 * allowing to extract the configuration very conveniently:
 * 	- Check for existence of a key (including error reporting)
 * 	- Check for the type of a key (including error reporting)
 * 	- Retrieve the value
 * 	- Handle "optional" values by specifying the optional value to be used
 * 	  if the configuration cannot be read.
 *
 * \section ConfigNew_Use Usage and Examples
 *
 * Without many words, here are the some examples.
 *
 * Example 1: Check for key type and existence.
 * \code
 * 	CConfigHelper hlp(configurationpath);
 * 	// Required Key: Will set fail=true if non-existent or wrong type
 *	fail |= !hlp.CheckConfig("datasource", Setting::TypeString);
 *	// Optional key: Will set fail=truel only if clearscreen is not boolean.
 *	fail |= !hlp.CheckConfig("clearscreen", Setting::TypeBoolean,
 *		true);
 * \endcode
 *
 *  Example 2: Retrieve key
 *  \code
 *  	CConfigHelper cfghlp(configurationpath);
 *	float interval;
 *	// This sets the value to 5.0 if not in the configuration:
 *	cfghlp.GetConfig("queryinterval", interval, 5.0f);
 *	// This would just set the value. if not configured, the value is left
 *	// alone.
 * 	cfghlp.GetConfig("queryinterval", interval);
 *  \endcode
 *
 *  \note C++ has strong types. So make sure, that everything matches.
 *  The above example would not compile, if you write "5.0" instead of "5.0f",
 *  as gcc would make a "double" and interval would be "float".
 *  This is also true for integers: int != unsigned int!
 *
 */

#ifndef CCONFIGHELPER_H_
#define CCONFIGHELPER_H_

#include <string>
#include <libconfig.h++>

#include "configuration/Registry.h"

using namespace std;

/** Encapsulates helper-functions needed for easier handling of configuration
 * issues.
 *
 * The class allows:
 * 	- easier checking of types
 * 	- easier access of values,
 * 	- easier access of optional values (by setting default values)
 *
 *
 *
 */
class CConfigHelper
{
public:
	/** Constructor
	 *
	 * \param configurationpath The path to the section of the
	 * configuration file we want to evaluate.
	 * (Usually you got this path by the factory generated your object)
	*/
	CConfigHelper( const string& configurationpath );
	virtual ~CConfigHelper();

	/** Basic checks on configuration keys, with the support for optional parameters
	 *
	 * This function checks for the parameter associated with the
	 * configuration-key (setting) for the existence (if not an optional parameter)
	 * and for type-correctness.
	 *
	 * For mandatory options (specified by parameter optional=false) the key
	 * must exists and be of the right datatype.
	 *
	 * Optional parameters needs not to be present, but when they are the
	 * datatype must be suitable.
	 *
	 * If printerr is given as true, a found configuration error
	 * (type, and existence of mandatory options)
	 * will be logged using the mainlogger.
	 *
	 * The function checks if the configuration-key (parameter setting)
	 * Mandatory options (optional=false, default) are checked for existence first,
	 * optional options are only checked if the type if ok.
	 * If printerr is given with true (default), the type of error is logged-.
	 *
	 * \returns true if the tests passed, else false
	 *
	 * \param setting configuration-key
	 * \param type expected type
	 * \param optional is it mandatory(false) or optional(true,default),
	 * \param printerr if true, explain the error to the main logger
	 * (default:false)
	 */
	bool CheckConfig( const string &setting, libconfig::Setting::Type type,
		bool optional = false, bool printerr = true );

	template<class T>

	/** Retrieves a configuration or set the config with a default value.
	 *
	 * \returns false, if the default value has been used.
	 */
	bool GetConfig( string const &setting, T &store, T defvalue )
	{
		libconfig::Setting & set
			= Registry::Instance().GetSettingsForObject(cfgpath);

		if (!set.lookupValue(setting, store)) {
			store = defvalue;
			return false;
		}
		return true;
	}

	/** Specialization of the GetConfig routine for the type unsigned-long.
	 * This is necessary as libconfig-1.4.8 removes long support.
	 *
	 * \returns false, if the default value has been used.
	 */
	bool GetConfig(string const &setting, unsigned long &store,
        unsigned long defvalue)
	{
	    unsigned int tmp;

        libconfig::Setting & set
            = Registry::Instance().GetSettingsForObject(cfgpath);

        if (!set.lookupValue(setting, tmp)) {
            store = defvalue;
            return false;
        }

        store = tmp;
        return true;
	}

    /** Specialization of the GetConfig routine for the type long.
     * This is necessary as libconfig-1.4.8 removes long support.
     * \returns false, if the default value has been used.
     */
	bool GetConfig(string const &setting, long &store,
            long defvalue)
    {
        int tmp;

        libconfig::Setting & set
            = Registry::Instance().GetSettingsForObject(cfgpath);

        if (!set.lookupValue(setting, tmp)) {
            store = defvalue;
            return false;
        }
        store = tmp;
        return true;
    }

    /** GetConfig when using a char-array as setting -- see ##GetConfig doc
      * \returns false, if the default value has been used.
     */
	template<class T>
	bool GetConfig( char const *setting, T &store, const T defvalue )
	{
		string s = setting;
		return GetConfig(s, store, defvalue);
	}

    /** GetConfig for mandatory settings.
      * \returns false, if the setting could not be retrieved.
     */
	template<class T>
	bool GetConfig( const string &setting, T &store )
	{
		libconfig::Setting & set
			= Registry::Instance().GetSettingsForObject(cfgpath);

		if (!set.lookupValue(setting, store)) {
			return false;
		}
		return true;
	}

    /** GetConfig for mandatory settings, flavour "char*"
      * \returns false, if the setting could not be retrieved.
     */
	template<class T>
	bool GetConfig( char const *setting, T &store )
	{
		string s = setting;
		return GetConfig(s, store);
	}

    /** GetConfig for mandatory unsigned long settings, for libconfig 1.4.8
      * \returns false, if the setting could not be retrieved.
     */
    bool GetConfig( const string &setting, unsigned long &store )
    {
        unsigned int tmp;
        libconfig::Setting & set
            = Registry::Instance().GetSettingsForObject(cfgpath);

        if (!set.lookupValue(setting, tmp)) {
            return false;
        }
        store = tmp;
        return true;
    }

    /** GetConfig for mandatory long settings, for libconfig 1.4.8
      * \returns false, if the setting could not be retrieved.
     */
    bool GetConfig( const string &setting, long &store )
    {
        int tmp;
        libconfig::Setting & set
            = Registry::Instance().GetSettingsForObject(cfgpath);

        if (!set.lookupValue(setting, tmp)) {
            return false;
        }
        store = tmp;
        return true;
    }


	/** Get the configuration for an array entry, identified by a index.
	 *
	 * See Libconfig's docs for how the arrays working.
	 *
	 * With this helper, you can simply query the array by its index.
	 *
	 * \param [in] setting Setting-id
	 * \param [in] index Index of the array
	 * \param [out] store place where config is placed.
	 *
	 * \return true on success, false if not an array, wrong type or index
	 * not existant.
	 *
	 */
	template<class T>
	bool GetConfigArray( const string& setting, int index, T &store )
	{
		libconfig::Setting & set
			= Registry::Instance().GetSettingsForObject(cfgpath);

		try {
			store = set[setting][index];

		} catch (libconfig::SettingNotFoundException &e) {
			return false;
		} catch (libconfig::SettingTypeException &e) {
			// TODO: Assert here?
			return false;
		}
		return true;
	}


	/** Get the configuration for an array entry, identified by a index.
	 * (string specalization) -> this is needed as libconfig would be ambigiius
	 * for char* and std::string.
	 *
	 * See Libconfig's docs for how the arrays working.
	 *
	 * With this helper, you can simply query the array by its index.
	 *
	 * \param [in] setting Setting-id
	 * \param [in] index Index of the array
	 * \param [out] store place where config is placed.
	 *
	 * \return true on success, false if not an array, wrong type or index
	 * not existant.
	 *
	 */
	bool GetConfigArray( const string& setting, int index, string &store )
	{
		libconfig::Setting & set
			= Registry::Instance().GetSettingsForObject(cfgpath);

		try {
			store = (const char *) set[setting][index];
		} catch (libconfig::SettingNotFoundException &e) {
			return false;
		} catch (libconfig::SettingTypeException &e) {
			// TODO: Assert here?
			return false;
		}
        return true;
	}

	/** this ugly helper is for the CHTHML Writer -- we are having a list which
	 *  embeddes a array. The entries are anonymous, that is without
	 * a destinctive name.
	 * I currently wonder how a better access function should look like ;-O)
	 * TODO make this more elegant and reusable...
	 *
	 * so, well, this function looks up an entry in the embedded array,
	 * with param i giving the row, index the column.
	 *
	 * \returns true on success (setting found, type ok),
	 *     false else (setting not found, type error)
	 */
	template<class T>
	bool GetConfigArray( const int i, int index, T &store )
	{
		libconfig::Setting & set
			= Registry::Instance().GetSettingsForObject(cfgpath);

		try {
			store = (const char *) set[i][index];
			return true;
		} catch (libconfig::SettingNotFoundException &e) {
			return false;
		} catch (libconfig::SettingTypeException &e) {
			// TODO: Assert here?
			return false;
		}
	}


    /** this ugly helper is for the CHTHML Writer, for types that are strings.#
     *
     * we are having a list which embeddes a array. The entries are anonymous, that is without
     * a destinctive name.
     * I currently wonder how a better access function should look like ;-O)
     * TODO make this more elegant and reusable...
     *
     * so, well, this function looks up an entry in the embedded array,
     * with param i giving the row, index the column.
     *
     * \returns true on success (setting found, type ok),
     *     false else (setting not found, type error)
     */

    bool GetConfigArray( const int i, int index, string &store )
    {
        libconfig::Setting & set
            = Registry::Instance().GetSettingsForObject(cfgpath);

        try {
            store = (const char *) set[i][index];
        } catch (libconfig::SettingNotFoundException &e) {
            return false;
        } catch (libconfig::SettingTypeException &e) {
            // TODO: Assert here?
            return false;
        }
        return true;
    }


private:
	string cfgpath;

};

#endif /* CCONFIGHELPER_H_ */
