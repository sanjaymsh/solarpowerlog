/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009-2011  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licensed
 as described in the file "COPYING".

 For this file (CSharedConnection.h), the license terms are:

 You can redistribute it and/or  modify it under the terms of the GNU Lesser
 General Public License (LGPL) as published by the Free Software Foundation;
 either version 3 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/**
 * \file CSharedConnection.h
 *
 * Sharing a connection between 2 or more inverters.
 *
 * This class "manages" the communication between two or more inverters,
 * talking to the same communication object.
 *
 * Every inverter sharing a connection uses a object of this class for its
 * communication.
 *
 * Exactly one of the objects is in the master role, owning the "real" communication
 * object ("master object") and the others are delegating the communication
 * to this master.
 *
 * The master schedules the sending and receiving of the telegrams, and notifies
 * the inverters about reception or errors....
 *
 * The receiving inverter has to take care about filtering out answers, as
 * there are inverters which do not employ a question-answer scheme, but just
 * answering.
 *
 * So, all inverters with pending receive commands will be informed.
 *
 * The slave object will only delegate the communication to the master and then
 * waits for the completion.
 *
 *  Created on: Sep 12, 2010
 *      Author: coldtobi
 */

#ifndef CSHAREDCONNECTION_H_
#define CSHAREDCONNECTION_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_COMMS_SHAREDCONNECTION

#include <assert.h>
#include "interfaces/IConnect.h"

class CSharedConnection: public IConnect
{
protected:
	friend class IConnectFactory;
	CSharedConnection(const string & configurationname);

public:

	virtual ~CSharedConnection();

	virtual bool Connect(ICommand *callback)
	{
		assert(concreteSharedConnection);
		return concreteSharedConnection->Connect(callback);
	}

	virtual bool Disconnect(ICommand *callback)
	{
		assert(concreteSharedConnection);
		return concreteSharedConnection->Disconnect(callback);
	}

	virtual void SetupLogger(const string& parentlogger, const string & = "");

	virtual bool Send(const char *tosend, unsigned int len, ICommand *callback)
	{
		assert(concreteSharedConnection);
		return concreteSharedConnection->Send(tosend, len, callback);
	}

	virtual bool Send(const string& tosend, ICommand *callback)
	{
		assert(concreteSharedConnection);
		return concreteSharedConnection->Send(tosend, callback);
	}

	virtual bool Receive(ICommand *callback)
	{
		assert(concreteSharedConnection);
		return concreteSharedConnection->Receive(callback);
	}

	virtual bool CheckConfig(void);

	virtual bool IsConnected(void)
	{
		assert(concreteSharedConnection);
		return concreteSharedConnection->IsConnected();
	}

private:

	bool CreateSharedConnectionObject();

	IConnect *concreteSharedConnection;

};

#endif

#endif /* CSHAREDCONNECTION_H_ */
