/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2015 Tobias Frost

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ----------------------------------------------------------------------------
 */

/*
 * CConfigCentralEntry.cpp
 *
 *  Created on: 04.01.2015
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <sstream>

#include "CConfigCentralEntry.h"
#include "CConfigCentral.h"


template <>
std::string CConfigCentralEntry<std::string>::GetConfigSnippet() const
{
    extern std::string _WrapForConfigSnippet(const std::string &description);

    std::string ret;
    // Wrap the desription
    ret = CConfigCentralHelpers::WrapForConfigSnippet(this->_description);

    std::stringstream ss;
    // print the optional / mandatory statement
    if (this->_optional) ss << "This setting is optional with a default value of " << this->_defvalue;
    else ss << "This setting is mandatory.\n";
    ret += CConfigCentralHelpers::WrapForConfigSnippet(ss.str());

    // make a nice example
    if (this->_optional) ret += "# ";
    ret += this->_setting + " = ";
    if (this->_optional) {
        std::stringstream ss;
        ss << '"' << this->_defvalue << '"';
        ret += ss.str();
    } else {
        ret += "\"<value>\"";
    }
    ret += ";\n";
    return ret;
}

template <>
std::string CConfigCentralEntry<bool>::GetConfigSnippet() const
{
    extern std::string _WrapForConfigSnippet(
        const std::string &description);

    std::string ret;
    // Wrap the desription
    ret = CConfigCentralHelpers::WrapForConfigSnippet(this->_description);
    // make a nice example

    std::stringstream ss;
     // print the optional / mandatory statement
    if (this->_optional) {
        ss << "This setting is optional with a default value of ";
        if (this->_defvalue) ss << "true";
        else ss << "false";
    }
    else ss << "This setting is mandatory.\n";
    ret += CConfigCentralHelpers::WrapForConfigSnippet(ss.str());

    if (_optional) ret += "# ";
    ret += _setting + " = ";
    if (_optional) {
        if (_defvalue) ret += "true";
        else ret += "false";
    } else {
        ret += "<true or false>";
    }
    ret += ";\n";
    return ret;
}
