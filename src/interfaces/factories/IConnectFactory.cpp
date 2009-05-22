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

#include "interfaces/factories/IConnectFactory.h"
#include "Connections/CConnectDummy.h"
#include "Connections/ConnectionTCP.h"

using namespace std;

IConnect * IConnectFactory::Factory(const string& type, const string &configurationpath)
{
	if(type == "TCP/IP") {
		return new CConnectTCP(configurationpath);
	}

	// TODO Implement other methos as soon as available.


	return new CConnectDummy(configurationpath);
}





