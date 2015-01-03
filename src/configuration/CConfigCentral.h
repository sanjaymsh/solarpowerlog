/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

 Copyright (C) 2009-2015 Tobias Frost

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ----------------------------------------------------------------------------
 */

/* \file CConfigCentral.h
 *
 *  Created on: 02.01.2015
 *      Author: tobi
 *
 */


#ifndef SRC_CONFIGURATION_CCONFIGCENTRAL_H_
#define SRC_CONFIGURATION_CCONFIGCENTRAL_H_


#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#include <list>
#include <string>

#include <boost/shared_ptr.hpp>

#include "ILogger.h"
#include "CConfigHelper.h"

#warning document me
class IConfigCentralEntry {

public:
    virtual ~IConfigCentralEntry() { }

    virtual bool CheckAndUpdateConfig(ILogger &logger, CConfigHelper &helper) const = 0;

};


#warning  document me
template<typename T>
class CConfigCentralEntry : public IConfigCentralEntry {

public:

    CConfigCentralEntry(const char* setting, const char* description, T &store) :
        _optional(false), _store(store), _setting(setting),
            _description(description)
    { }

    // Constructor for optional parameters.
    CConfigCentralEntry(const char* setting, const char* description, T &store,
        T defaultvalue) :
        _optional(true), _store(store), _defvalue(defaultvalue),
            _setting(setting), _description(description)
    { }

    virtual ~CConfigCentralEntry() { }

    virtual bool CheckAndUpdateConfig(ILogger &logger, CConfigHelper &helper) const {

        if (_optional) {
            return helper.CheckAndGetConfig(_setting.c_str(), _store, _defvalue, &logger);
        }
        else {
            return helper.CheckAndGetConfig(_setting, _store, &logger);
        }
    }

private:
    bool _optional;
    T &_store;
    T _defvalue;
    std::string _setting;
    std::string _description;

};

/** Configuration Helper Class for better checking and documentating
 * configuration options for solarpowerlog
 *
 * It is cumbersome to keep documentation and code in sync. This class
 * should couple documentation with the code, and also make the checking of
 * the configuration and configuration error reporting more straight forward.
 *
 * The class is designed to be instantiated within the class to be configured,
 * so that the values can be directly placed into the class' variables (via
 * pointers)
 *
 */
class CConfigCentral {

public:

    CConfigCentral() {};

    virtual ~CConfigCentral() {

    }

    /// Add an option (mandatory option)
    template<typename T>
    CConfigCentral& operator()(const char* parameter, const char* description,
        T &store)
    {
        boost::shared_ptr<IConfigCentralEntry>
        p((IConfigCentralEntry*) new CConfigCentralEntry<T>(parameter,
                description, store));
        l.push_back(p);
        return *this;
    }

    /// Add an option with defaultvalue (thus optional parameter)
    template<typename T>
    CConfigCentral& operator()(const char* parameter, const char* description,
        T &store, T defaultvalue)
    {
        boost::shared_ptr<IConfigCentralEntry>
        p((IConfigCentralEntry*) new CConfigCentralEntry<T>(parameter,
                description, store, defaultvalue));
        l.push_back(p);

        return *this;
    }

    /** Parse the configuration and use the supplied logger to print errors.
     *
     * Store values into their targets (if given)
     *
     * @return true on success, false on config errors.
     */
    bool CheckConfig(ILogger &logger, const std::string &configpath);

private:
    std::list<boost::shared_ptr< IConfigCentralEntry> > l;


};

#endif /* SRC_CONFIGURATION_CCONFIGCENTRAL_H_ */
