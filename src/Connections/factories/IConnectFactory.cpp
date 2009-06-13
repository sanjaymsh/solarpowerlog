/* ----------------------------------------------------------------------------
   solarpowerlog
   Copyright (C) 2009  Tobias Frost

   This file is part of solarpowerlog.

   Solarpowerlog is free software; However, it is dual-licenced
   as described in the file "COPYING".

   For this file (IConnectFactory.cpp), the license terms are:

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


/** \file IConnectFactory.cpp
 *
 *  Created on: May 16, 2009
 *      Author: tobi
 */
#include "configuration/Registry.h"

#include "Connections/factories/IConnectFactory.h"
#include "Connections/CConnectDummy.h"
#include "Connections/CConnectTCP.h"

#include <libconfig.h++>
#include "Connections/CConnectTCPAsio.h"


using namespace std;

IConnect * IConnectFactory::Factory(const string &configurationpath)
{

	libconfig::Setting &set = Registry::Instance().GetSettingsForObject(configurationpath);
	string type;

	set.lookupValue("comms", type);

	if(type == "COMMONCPP::TCP/IP") {
		// this one was the first attempt..
		// unfortnuatly this implementation did not work
		// well.
		// (see bug #532228 of the Debian BTS)
		return new CConnectTCP(configurationpath);
	}

	if(type == "TCP/IP") {
		return new CConnectTCPAsio(configurationpath);
	}


	// TODO Implement other methos as soon as available.


	return new CConnectDummy(configurationpath);
}





