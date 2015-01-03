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

#define WRAP_AT (79)


/** Interface class for config entries.
 *
 */
class IConfigCentralEntry {
public:
    virtual ~IConfigCentralEntry() { }

    /** Check the config entry and if ok update the target value.
     *
     * @param logger where to log errors
     * @param helper where to retrive the settings.
     * @return true if setting ok, false if something is wrong. The reason is
     *    logged using LOGERROR(logger,...)
     */
    virtual bool CheckAndUpdateConfig(ILogger &logger, CConfigHelper &helper) const = 0;

    /** Get the configuration options pretty-printed to automatically generate
     * config snippets.
     */
    virtual std::string GetConfigSnippet() const = 0;

    /** Get the configuration options pretty-printed to suitable for help2man
     */
    // virtual std::string& GetHelp() const = 0;

};

/** Class for a description, without parameter.
 *
 * note: to get a
 */
class CConfigCentralEntryText : public IConfigCentralEntry {
public:
    CConfigCentralEntryText(const char *parameter, const char *description, const char *example) {
        if (parameter) _parameter = parameter;
        if (example) _example = example;
        assert(description);
        _description = description;
    }

    virtual bool CheckAndUpdateConfig(ILogger &, CConfigHelper &) const {
        return true;
    }

    virtual std::string GetConfigSnippet() const;

    virtual ~CConfigCentralEntryText() { }

private:
    std::string _example;
    std::string _parameter;
    std::string _description;
};


/*** Single setting/configuration entry.
 *
 */
template<typename T>
class CConfigCentralEntry : public IConfigCentralEntry {

public:

    /** Constructor for mandatory parameters
     *
     * @param setting which setting
     * @param description description for the setting
     * @param store where to store the parsed value
     */
    CConfigCentralEntry(const char* setting, const char* description, T &store) :
        _optional(false), _store(store), _setting(setting),
            _description(description)
    { }

    /** Constructor for optional parameters
     *
     * @param setting which setting
     * @param description description for the setting
     * @param store where to store the parsed value
     * @param defaultvalue which value to use when the parameter was not found
     */
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

    std::string GetConfigSnippet() const
    {
        extern std::string _WrapForConfigSnippet(
            const std::string &description);
        extern std::string _WrapForConfigSnippet(
            const std::string &description);

        std::string ret;
        // Wrap the desription
        ret = _WrapForConfigSnippet(this->_description);

        std::stringstream ss;
        // print the optional / mandatory statement
        if (this->_optional) ss << "This setting is optional with a default value of " << this->_defvalue;
        else ss << "This setting is mandatory.\n";
        ret += _WrapForConfigSnippet(ss.str());

        // make a nice example
        if (this->_optional) ret += "# ";
        ret += this->_setting + " = ";
        if (this->_optional) {
            std::stringstream ss;
            ss << this->_defvalue;
            ret += ss.str();
        } else {
            ret += "<value>";
        }
        ret += ";\n";
        return ret;
    }

protected:
    bool _optional;
    T &_store;
    T _defvalue;
    std::string _setting;
    std::string _description;
};

/*** Single setting/configuration entry.
 *
 */
template<typename T>
class CConfigCentralEntryRangeCheck : public CConfigCentralEntry<T> {

public:

    /** Constructor for mandatory parameters with range check
     *
     * @param setting which setting
     * @param description description for the setting
     * @param store where to store the parsed value
     * @param minimum allowed value (including)
     * @param maximum allowed value (including)
     */
    CConfigCentralEntryRangeCheck(const char* setting, const char* description,
        T &store, const T &minimum, const T &maximum) :
            CConfigCentralEntry<T>(setting, description, store),
            _min(minimum), _max(maximum)
     { }

    /** Constructor for optional parameters with range check
     *
     * @param setting which setting
     * @param description description for the setting
     * @param store where to store the parsed value
     * @param defaultvalue which value to use when the parameter was not found
     * @param minimum allowed value (including)
     * @param maximum allowed value (including)
     */
    CConfigCentralEntryRangeCheck(const char* setting, const char* description,
        T &store, const T &defaultvalue, const T &minimum, const T &maximum) :
            CConfigCentralEntry<T>(setting, description, store, defaultvalue),
            _min(minimum), _max(maximum)

    { }

    virtual ~CConfigCentralEntryRangeCheck() { }

    virtual bool CheckAndUpdateConfig(ILogger &logger,
        CConfigHelper &helper) const
    {
        bool ret;
        T old = this->_store;

        ret = CConfigCentralEntry<T>::CheckAndUpdateConfig(logger, helper);

        if (!ret) {
            this->_store = old;
            return false;
        }

        if (this->_store > _max || this->_store < _min) {
            LOGERROR(logger, "Setting " << this->_setting <<
                " is out of range (allowed range: " << _min << " to " << _max);
            this->_store = old;
            return false;
        }
        return true;
    }

    virtual std::string GetConfigSnippet() const
    {
        extern std::string _WrapForConfigSnippet(
            const std::string &description);

        std::string ret;
        // Wrap the desription
        ret = _WrapForConfigSnippet(this->_description);

        std::stringstream ss;
        // print the range
        ss << "# Valid values are from " << this->_min << " to " << this->_max << ".\n";
        // print the optional / mandatory statement
        if (this->_optional) ss << "This setting is optional with a default value of " << this->_defvalue;
        else ss << "This setting is mandatory.\n";
        ret += _WrapForConfigSnippet(ss.str());

        // make a nice example
        if (this->_optional) ret += "# ";
        ret += this->_setting + " = ";
        if (this->_optional) {
            std::stringstream ss;
            ss << this->_defvalue;
            ret += ss.str();
        } else {
            ret += "<value>";
        }
        ret += ";\n";
        return ret;
    }

private:
    T _min;
    T _max;
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


    /** Add an textual entry (which can be used when dumping the help
     * to have an header, prequel text or like or parameters which are
     * (reported) aliases to others (like the typo "manufactor" instead of
     * "manufacturer")
     *
     * \param parameter parameter linked to this entry. Might be NULL.
     * \param description description, text...
     */
    CConfigCentral& operator()(const char* parameter, const char *description,
        const char *example = NULL)
    {
        boost::shared_ptr<IConfigCentralEntry> p((IConfigCentralEntry*)
            new CConfigCentralEntryText(parameter, description, example));
            l.push_back(p);
            return *this;
        }


    /** Add an setting describing entry (mandatory version)
     *
     * @param parameter setting's name
     * @param description setting's description
     * @param store where to store the value
     * @return object, so that the operator can be cascaded.
     */
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

    /** Add an setting describing entry (optional version with default value)
      *
      * @param parameter setting's name
      * @param description setting's description
      * @param store where to store the value
      * @param defaultvalue to use when the setting was not found.
      * @return object, so that the operator can be cascaded.
      */
    template<typename T>
    CConfigCentral& operator()(const char* parameter, const char* description,
        T &store, const T &defaultvalue)
    {
        boost::shared_ptr<IConfigCentralEntry>
        p((IConfigCentralEntry*) new CConfigCentralEntry<T>(parameter,
                description, store, defaultvalue));
        l.push_back(p);

        return *this;
    }

    /** Add an setting describing entry (mandatory version) with range-check
     *
     * @param parameter setting's name
     * @param description setting's description
     * @param store where to store the value
     * @param minimum range limit (including)
     * @param maximum range limit (including
     * @return object, so that the operator can be cascaded.
     */
    template<typename T>
    CConfigCentral& operator()(const char* parameter, const char* description,
            T &store, const T &minimum, const T &maximum)
        {
        boost::shared_ptr<IConfigCentralEntry> p(
            (IConfigCentralEntry*)new CConfigCentralEntryRangeCheck<T>(
                parameter, description, store, minimum, maximum));
        l.push_back(p);
        return *this;
        }

    /** Add an setting describing entry (optinal version) with range-check
     *
     * @param parameter setting's name
     * @param description setting's description
     * @param store where to store the value
     * @param defaultvalue to use when setting has not been found
     * @param minimum range limit (including)
     * @param maximum range limit (including
     * @return object, so that the operator can be cascaded.
     */
    template<typename T>
    CConfigCentral& operator()(const char* parameter, const char* description,
        T &store, const T &defaultvalue, const T &minimum, const T &maximum)
    {
        boost::shared_ptr<IConfigCentralEntry> p(
            (IConfigCentralEntry*)new CConfigCentralEntryRangeCheck<T>(
                parameter, description, store, defaultvalue, minimum, maximum));
        l.push_back(p);

        return *this;
    }


    /** Parse the configuration and use the supplied logger to print errors.
     *
     * Store values into their targets (if given)
     *
     * @return true on success, false on config errors.
     */
    bool CheckConfig(ILogger &logCConfigCentralEntryger, const std::string &configpath);

    /** Get a configuration snippet */
    std::string GetConfigSnippet();


private:
    std::list<boost::shared_ptr< IConfigCentralEntry> > l;
};

#endif /* SRC_CONFIGURATION_CCONFIGCENTRAL_H_ */
