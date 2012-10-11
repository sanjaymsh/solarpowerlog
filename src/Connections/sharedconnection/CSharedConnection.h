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
#include "Connections/interfaces/IConnect.h"

class CSharedConnection: public IConnect
{
protected:
	friend class IConnectFactory;
	CSharedConnection(const string & configurationname);

public:

	virtual ~CSharedConnection();

	virtual void Connect(ICommand *callback)
	{
		assert(concreteSharedConnection);
		concreteSharedConnection->Connect(callback);
	}

	virtual void Disconnect(ICommand *callback)
	{
		assert(concreteSharedConnection);
		concreteSharedConnection->Disconnect(callback);
	}

	virtual void SetupLogger(const string& parentlogger, const string & = "");

	virtual void Send(ICommand *cmd) {
		assert(concreteSharedConnection);
		concreteSharedConnection->Send(cmd);
	}

	virtual void Receive(ICommand *callback)
	{
		assert(concreteSharedConnection);
		concreteSharedConnection->Receive(callback);
	}

	virtual bool CheckConfig(void);

	virtual bool IsConnected(void)
	{
		assert(concreteSharedConnection);
		return concreteSharedConnection->IsConnected();
	}

    virtual bool AbortAll() {
        assert(concreteSharedConnection);
        return concreteSharedConnection->AbortAll();
    }


private:

	bool CreateSharedConnectionObject();

	IConnect *concreteSharedConnection;

};

#endif

#endif /* CSHAREDCONNECTION_H_ */
