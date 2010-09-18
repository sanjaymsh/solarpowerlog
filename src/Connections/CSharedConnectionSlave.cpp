/*
 * CConnectSlave.cpp
 *
 *  Created on: Sep 13, 2010
 *      Author: tobi
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_COMMS_SHAREDCONNECTION

#include "CSharedConnectionSlave.h"


CSharedConnectionSlave::CSharedConnectionSlave(
		const string & configurationname) :
	IConnect(configurationname)
{

}

CSharedConnectionSlave::~CSharedConnectionSlave()
{

}

bool CSharedConnectionSlave::Connect(ICommand *callback)
{

}

bool CSharedConnectionSlave::Disconnect(ICommand *callback)
{

}

void CSharedConnectionSlave::SetupLogger(const string& parentlogger,
		const string &)
{

}

bool CSharedConnectionSlave::Send(const char *tosend, unsigned int len,
		ICommand *callback)
{

}

bool CSharedConnectionSlave::Send(const string& tosend, ICommand *callback)
{

}

bool CSharedConnectionSlave::Receive(string &wheretoplace, ICommand *callback)
{

}

bool CSharedConnectionSlave::CheckConfig(void)
{

}

bool CSharedConnectionSlave::IsConnected(void)
{

}

#endif
