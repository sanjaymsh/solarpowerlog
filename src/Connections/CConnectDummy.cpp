/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2009-2012 Tobias Frost

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

/** \file CConnectDummy.cpp
 *
 *  Created on: May 22, 2009
 *      Author: tobi
 */

#include "CConnectDummy.h"
#include <iostream>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

using namespace std;

CConnectDummy::CConnectDummy(const string &configurationname)
: IConnect(configurationname)
{
}

CConnectDummy::~CConnectDummy() {
	// TODO Auto-generated destructor stub
}

bool CConnectDummy::CheckConfig(void)
{
	LOGERROR(this->logger, "Unknown communication method:. Please check your comms= setting.");
	return false;
}


