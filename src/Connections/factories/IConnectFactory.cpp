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
 *  \date Created on: May 16, 2009
 *  \author: Tobias Frost (coldtobi)
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#include "configuration/Registry.h"
#include "configuration/CConfigHelper.h"
#include "Connections/factories/IConnectFactory.h"
#include "Connections/CConnectDummy.h"

#include <libconfig.h++>

#ifdef HAVE_COMMS_ASIOTCPIO
#include "Connections/CConnectTCPAsio.h"
#endif

#ifdef HAVE_COMMS_ASIOSERIAL
#include "Connections/CConnectSerialAsio.h"
#endif

using namespace std;

/** Facortry for generation of connection methods.
 * Give it the configurationpath, and out of the config, it will
 * generate the right class.
 *
 * If the class is not known, it will return a dummy connection class.
 * So you can also create inverters or derived classes without commms. */
IConnect * IConnectFactory::Factory( const string &configurationpath )
{
	string type = "";
	CConfigHelper cfghelper(configurationpath);
	cfghelper.GetConfig("comms",type);

#ifdef HAVE_COMMS_ASIOTCPIO
	if (type == "TCP/IP") {
		return new CConnectTCPAsio(configurationpath);
	}
#endif
#ifdef HAVE_COMMS_ASIOSERIAL
	if (type == "RS2xx") {
		return new CConnectSerialAsio(configurationpath);
	}
#endif

	return new CConnectDummy(configurationpath);
}

