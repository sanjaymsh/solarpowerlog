/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2010-2012 Tobias Frost

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ----------------------------------------------------------------------------
 */
/*
 * CSharedConnectionMaster.h
 *
 *  Created on: Sep 13, 2010
 *      Author: tobi
 */

#ifndef CSHAREDCONNECTIONMASTER_H_
#define CSHAREDCONNECTIONMASTER_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_COMMS_SHAREDCONNECTION

#include <list>
#include <semaphore.h>

#include "interfaces/IConnect.h"
#include "Connections/CAsyncCommand.h"
#include "patterns/ICommandTarget.h"
#include "patterns/ICommand.h"
#include "CSharedConnectionSlave.h"


// Token inserted by this or the slave class to specify individual timeouts.
// At this timestamp, the command can be considered timed-out.
#define ICONNECT_TOKEN_TIMEOUTTIMESTAMP "CSharedConnection_Timeout"

#define ICONNECT_TOKEN_PRV_ORIGINALCOMMAND "CSharedConnection_OrgiginalWork"

#define SHARED_CONN_MASTER_DEFAULTTIMEOUT (3000UL)

class CSharedConnectionMaster: public IConnect, ICommandTarget
{

protected:
	friend class CSharedConnection;
	friend class CSharedConnectionSlave;
	CSharedConnectionMaster(const string & configurationname);

public:
	virtual ~CSharedConnectionMaster();

	void ExecuteCommand(const ICommand *Command);

protected:

	virtual void Connect(ICommand *callback);

	virtual void Disconnect(ICommand *callback);

	virtual void SetupLogger(const string& parentlogger, const string & = "");

	virtual void Send(ICommand *cmd);

	virtual void Receive(ICommand *callback);

	virtual bool CheckConfig(void);

	virtual bool IsConnected(void);

    virtual bool AbortAll();

private:
	IConnect *connection;

	list<ICommand*> readcommands;

	// When is the current receive scheduled to timeout?
	boost::posix_time::ptime readtimeout;

};

#endif

#endif /* CSHAREDCONNECTIONMASTER_H_ */
