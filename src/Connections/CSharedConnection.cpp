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
		LOGFATAL(logger,"Configuration Error: Sharedconnection_type not defined. Must be master or slave.");
		return false;
	}

	if ( !CreateSharedConnectionObject() ) {
		LOGFATAL(logger,"Configuration Error: Sharedconnection_type must be master or slave.");
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
