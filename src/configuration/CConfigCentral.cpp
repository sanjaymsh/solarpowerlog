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

/** \file CConfigCentral.cpp
 *
 *  Created on: 02.01.2015
 *      Author: tobi
 */

#include "CConfigCentral.h"


std::string _WrapForConfigSnippet(const std::string &description) {

    std::string ret;
    std::string tmp = description;

    // the parsed snipped should look like
    // # <description wrapped at 80, manually wrap at \n
    // # manually remove double spaces and leading spaces.
    // # parameter = <your-value>

    // remove trailing white spaces (this ensures that the last char is not
    // a whitespace, so we can safely look ahead later.)
    // first remove trailing spaces / newlines.
    std::string whitespaces(" \t\f\v\n\r");
    std::size_t found = tmp.find_last_not_of(whitespaces);
    if (found != std::string::npos) tmp.erase(found + 1);
    else tmp.clear();            // is all whitespace?

    // iterate through string to remove double spaces
    size_t cursor = 0;
    std::string doublespace = "  ";
    while (1) {
        cursor = tmp.find(doublespace, cursor);
        if (std::string::npos == cursor) break;
        tmp.erase(cursor, 1);
    }

    cursor = 0;
    size_t column = 0;
    size_t len = tmp.length() - 1;
    // iterate through remaining string.
    while (len > cursor) {
        size_t s;
        // Add trailing "# "
        if (0 == column && len > cursor) {
            ret += "# ";
            column = 2;
        }

        if (tmp[cursor] == ' ') {
            cursor++;
            continue;
        }

        // look for newlines that shold break actual line
        s = tmp.find('\n', cursor);
        if (std::string::npos != s && (s - cursor) < (WRAP_AT - column)) {
            ret += tmp.substr(cursor, s - cursor + 1);
            column = 0;
            cursor = s + 1;
            continue;
        }

        // is the remaining line shorter than we would need to break anyway?
        s = len - cursor;
        if (s < (WRAP_AT - column)) {
            ret += tmp.substr(cursor);
            ret += '\n';
            break;
        }

        // find the spot to break
        // cursor = last character parsed
        // column = horizontal position
        s = tmp.find_last_of(whitespaces, cursor + WRAP_AT - column);
        // s == npos means that we've got a monster string without spaces.
        // also if s is smaller than the current cursor position.
        if (std::string::npos == s || s < cursor) {
            s = tmp.find_first_not_of(whitespaces, cursor);
            if (s != std::string::npos) {
                ret += tmp.substr(cursor, s - cursor);
                cursor = s;
                continue;
            } else {
                ret += tmp.substr(cursor);
                ret += "\n";
                break;
            }
        } else {
            // s is > cursor and our wrap position
            ret += tmp.substr(cursor, s - cursor);
            cursor = s;
            ret += "\n";
            column = 0;
        }
    }

    return ret;

}

std::string CConfigCentralEntryText::GetConfigSnippet() const
{
   std::string ret =  _WrapForConfigSnippet(_description);

   if (!_parameter.empty() && !_example.empty()) {
       ret += "# " + _parameter + " = " + _example +";\n";
   }

   return ret;
}



bool CConfigCentral::CheckConfig(ILogger& logger, const std::string &configpath)
{
    bool ret = true;
    CConfigHelper helper(configpath);
    std::list<boost::shared_ptr<IConfigCentralEntry> >::iterator it;
    for (it = l.begin(); it != l.end(); it++) {
        if (!(*it)->CheckAndUpdateConfig(logger,helper)) ret = false;
    }

    return ret;
}

std::string CConfigCentral::GetConfigSnippet()
{
    std::string ret;

    std::list<boost::shared_ptr<IConfigCentralEntry> >::iterator it;
    for (it = l.begin(); it != l.end(); it++) {
        ret += (*it)->GetConfigSnippet();
        ret += '\n';
    }

    return ret;
}

template <>
std::string CConfigCentralEntry<std::string>::GetConfigSnippet() const
{
    extern std::string _WrapForConfigSnippet(const std::string &description);

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
    ret = _WrapForConfigSnippet(this->_description);
    // make a nice example

    std::stringstream ss;
     // print the optional / mandatory statement
    if (this->_optional) {
        ss << "This setting is optional with a default value of ";
        if (this->_defvalue) ss << "true";
        else ss << "false";
    }
    else ss << "This setting is mandatory.\n";
    ret += _WrapForConfigSnippet(ss.str());

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
