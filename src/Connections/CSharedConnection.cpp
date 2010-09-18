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
	CConfigHelper cfg(ConfigurationPath);
	std::string s;

	if (!cfg.GetConfig("SharedConnectionType", s))
		return false;

	if (s == "master") {
		concreteSharedConnection = new CSharedConnectionMaster(
				this->ConfigurationPath);
	} else if (s == "slave") {
		concreteSharedConnection = new CSharedConnectionSlave(
				this->ConfigurationPath);
	} else
		return false;
}

bool CSharedConnection::CheckConfig(void)
{

	bool fail = false;
	CConfigHelper cfg(ConfigurationPath);
	std::string s;

	if (!cfg.CheckConfig("SharedConnectionType", Setting::TypeString)) {
		fail = true;
	} else {
		cfg.GetConfig("SharedConnectionType", s);
		if (s == "slave") {
			LOGDEBUG(this->logger,"SharedConnection slave requested.");
		} else if (s == "master") {
			LOGDEBUG(this->logger,"SharedConnection master requested.");
		} else {
			LOGFATAL(this->logger,"SharedConnection type must be master or slave.");
			return false;
		}
	}

	if (fail)
		return false;

	if (!concreteSharedConnection)
	if (!CreateSharedConnectionObject()) return false;

	if (!concreteSharedConnection->CheckConfig())
		return false;

	return true;

}

void CSharedConnection::SetupLogger(const string& parentlogger,
		const string &)
{
	IConnect::SetupLogger(parentlogger, "Comms_SharedConnection");

	if (!concreteSharedConnection)
		CreateSharedConnectionObject();

	if (!concreteSharedConnection)
		concreteSharedConnection->SetupLogger(parentlogger
				+ ".Comms_SharedConnection");
}

#endif
