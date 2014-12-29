/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009-2011  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CSharedConnection.cpp), the license terms are:

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
 * CSharedConnection.cpp
 *
 *  Created on: Sep 12, 2010
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_COMMS_SHAREDCONNECTION

#include "CSharedConnection.h"
#include "configuration/CConfigHelper.h"
#include "CSharedConnectionMaster.h"
#include "CSharedConnectionSlave.h"

using namespace libconfig;

CSharedConnection::CSharedConnection(const string & configurationname) :
	IConnect(configurationname)
{
	concreteSharedConnection = NULL;
}

CSharedConnection::~CSharedConnection()
{
	if (concreteSharedConnection)
		delete concreteSharedConnection;
}

bool CSharedConnection::CreateSharedConnectionObject()
{
	if (concreteSharedConnection) return true;

	CConfigHelper cfg(ConfigurationPath);
	std::string s;

	if (!cfg.GetConfig("sharedconnection_type", s))
		return false;

	if (s == "master") {
		LOGDEBUG(this->logger,"Shared connection master requested.");
		concreteSharedConnection = new CSharedConnectionMaster(
				this->ConfigurationPath);
	} else if (s == "slave") {
		LOGDEBUG(this->logger,"Shared connection slave requested.");
		concreteSharedConnection = new CSharedConnectionSlave(
				this->ConfigurationPath);
	} else {
		LOGERROR(this->logger,"Shared connection; Slave or master?");
		return false;
	}

	concreteSharedConnection->SetupLogger(this->logger.getLoggername(),
			"SharedTarget");

	return true;

}

bool CSharedConnection::CheckConfig(void)
{

	bool fail = false;
	CConfigHelper cfg(ConfigurationPath);
	std::string s;

	if (!cfg.GetConfig("sharedconnection_type", s)) {
		LOGERROR(logger,"Configuration Error: Sharedconnection_type not defined. Must be master or slave.");
		return false;
	}

	if ( !CreateSharedConnectionObject() ) {
		LOGERROR(logger,"Configuration Error: Sharedconnection_type must be master or slave.");
		return false;
	}


	if (fail)
		return false;

	if (!concreteSharedConnection->CheckConfig())
		return false;

	return true;

}

void CSharedConnection::SetupLogger(const string& parentlogger, const string &)
{
	IConnect::SetupLogger(parentlogger, "Comms_SharedConnection");
}

#endif
