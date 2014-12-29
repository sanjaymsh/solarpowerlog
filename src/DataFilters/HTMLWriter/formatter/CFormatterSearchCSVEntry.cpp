/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009-2011  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CFormatterSearchCSVEntry.cpp), the license terms are:

 You can redistribute it and/or modify it under the terms of the GNU
 General Public License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */
/*
 * CFormatterSearchCSVEntry.cpp
 *
 *  Created on: Jan 5, 2010
 *      Author: tobi
 */

#include "DataFilters/HTMLWriter/formatter/CFormatterSearchCSVEntry.h"

#include <sstream>

using namespace std;

bool CFormatterSearchCSVEntry::Format(const std::string & what,
		std::string & store, const std::vector<std::string> & parameters)
{
	// the task is to get a comma-seperated value by what
	// and extract the position for the element given by the parameter
	// [2] and store the number in store.
	std::string s;

	try {
		s = parameters[2];
	} catch (...) {
		return false;
	}

	// What we got:
	// value, value2, value3
	// We count the seperators, and we are set.

	size_t pos = what.find(s);

	if (pos == std::string::npos)
		return false;

	int count = 0;
	for (size_t i = 0; i < pos; i++) {
		if (what[i] == ',' && (i == 0 || what[i - 1] != '\\')) {
			count++;
		}
	}

	std::stringstream ss;
	ss << count;
	ss >> store;

	return true;
}

CFormatterSearchCSVEntry::CFormatterSearchCSVEntry()
{
	// TODO Auto-generated constructor stub

}

CFormatterSearchCSVEntry::~CFormatterSearchCSVEntry()
{
	// TODO Auto-generated destructor stub
}
