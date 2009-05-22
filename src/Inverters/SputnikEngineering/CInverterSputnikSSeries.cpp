/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CInverterSputnikSSeries.cpp), the license terms are:

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

/** \file CInverterSputnikSSeries.cpp
 *
 *  Created on: May 21, 2009
 *      Author: tobi
 */

#include "configuration/Registry.h"

#include "CInverterSputnikSSeries.h"

#include <libconfig.h++>
#include <iostream>

using namespace std;

CInverterSputnikSSeries::CInverterSputnikSSeries(const string &name, const string & configurationpath)
: IInverterBase::IInverterBase(name, configurationpath)
{
	// Add the capabilites that this inverter has
	// Note: The "must-have" ones are already instanciated by the base class constructor.
}

CInverterSputnikSSeries::~CInverterSputnikSSeries() {
	// TODO Auto-generated destructor stub
}

bool CInverterSputnikSSeries::CheckConfig()
{
	string setting;
	string str;

	bool ret = true;
	// Check, if we have enough informations to work on.
	libconfig::Setting &set = Registry::Instance().GetSettingsForObject(configurationpath);

	setting = "comms";
	if (! set.exists(setting) || !set.getType() ==  libconfig::Setting::TypeString) {
		cerr << "Setting " << setting << " in " << configurationpath << "."
			<< name << " missing or of wrong type (wanted a string)" << endl;
		ret = false;
	}

	// Check config of the component, if already instanciated.
	if (connection ) {
		ret = connection->CheckConfig();
	}

	setting = "commadr";
	if (! set.exists(setting) || !set.getType() ==  libconfig::Setting::TypeInt) {
		cerr << "Setting " << setting << " in " << configurationpath << "."
			<< name << " missing or of wrong type (wanted a integer)" << endl;
		ret = false;
	}

	return ret;
}


