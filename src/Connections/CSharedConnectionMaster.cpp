/*
 * CSharedConnectionMaster.cpp
 *
 *  Created on: Sep 13, 2010
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_COMMS_SHAREDCONNECTION

#include "CSharedConnectionMaster.h"

CSharedConnectionMaster::CSharedConnectionMaster(
		const string & configurationname) :
	IConnect(configurationname)
{

}

CSharedConnectionMaster::~CSharedConnectionMaster()
{

}

bool CSharedConnectionMaster::Connect(ICommand *callback)
{

}

bool CSharedConnectionMaster::Disconnect(ICommand *callback)
{

}

void CSharedConnectionMaster::SetupLogger(const string& parentlogger,
		const string &)
{

}

bool CSharedConnectionMaster::Send(const char *tosend, unsigned int len,
		ICommand *callback)
{

}

bool CSharedConnectionMaster::Send(const string& tosend, ICommand *callback)
{

}

bool CSharedConnectionMaster::Receive(string &wheretoplace, ICommand *callback)
{

}

bool CSharedConnectionMaster::CheckConfig(void)
{

}

bool CSharedConnectionMaster::IsConnected(void)
{

}

#endif
