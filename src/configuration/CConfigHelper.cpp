/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CConfigHelper.cpp), the license terms are:

 You can redistribute it and/or modify it under the terms of the GNU
 General Public License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This programm is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/** \file CConfigHelper.cpp
 *
 * \date Jul 4, 2009
 * \author Tobias Frost
 */

#include <iostream>

#include "configuration/CConfigHelper.h"
#include "configuration/Registry.h"

using namespace std;
using namespace libconfig;

CConfigHelper::CConfigHelper( const string& configurationpath )
{
	cfgpath = configurationpath;
}

CConfigHelper::~CConfigHelper()
{
	// TODO Auto-generated destructor stub
}

bool CConfigHelper::CheckConfig( const string & setting,
	libconfig::Setting::Type type, bool optional, bool printerr )
{
	libconfig::Config* cfg = Registry::Instance().Configuration();
	string reason = "";

	string tmp = cfgpath + "." + setting;

	if (!cfg->exists(tmp)) {
		if (optional) {
			return true;
		}
		if (printerr)
			reason = "was not found";
	} else {
		libconfig::Setting & set = cfg->lookup(tmp);
		if (printerr)
			reason = "is of wrong type.";

		switch (type) {
		case Setting::TypeInt:
		case Setting::TypeFloat:
		case Setting::TypeInt64:
		{
			if (set.isNumber())
				return true;
			break;
		}
		default:
			if (set.getType() == type) {
				return true;
			}
		}
	}

	if (printerr) {
		string name = cfgpath;
		cfg->lookupValue(cfgpath + ".name", name);
		cerr << "Setting " << setting << " of " << name << " " << " "
			<< reason << endl;
	}
	return false;
}
