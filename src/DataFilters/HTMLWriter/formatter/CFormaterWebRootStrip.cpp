/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2010-2012 Tobias Frost

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
 * CFormaterWebRootStrip.cpp
 *
 *  Created on: Jan 4, 2010
 *      Author: tobi
 */

#include "DataFilters/HTMLWriter/formatter/IFormater.h"
#include "DataFilters/HTMLWriter/formatter/CFormaterWebRootStrip.h"
#include "configuration/CConfigHelper.h"

CFormaterWebRootStrip::CFormaterWebRootStrip()
{
}

CFormaterWebRootStrip::~CFormaterWebRootStrip()
{
	// TODO Auto-generated destructor stub
}

bool CFormaterWebRootStrip::Format(const std::string &what, std::string &store,
		const std::vector<std::string> &parameters)
{
	std::string strip;
	size_t pos;

	if (parameters.size() < 3) {
		strip = "/var/www";
	} else {
		strip = parameters[2];
	}

	// basic check: what must be bigger than strip.
	if (strip.length() > what.length())
		return false;

	// make sure that the string is there
	pos = what.find(strip);

	if (pos != std::string::npos) {
		store = what.substr(strip.length(), std::string::npos);
		return true;
	}

	return false;
}

