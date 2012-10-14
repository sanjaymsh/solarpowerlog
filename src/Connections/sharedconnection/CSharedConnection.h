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
 \section CSC_DESIGN Design of the (new) sharedcomms

 The sharedcomms purpose is to share a communication object (doing the talk over
 one physical line) and keep track that this communication follow the rules the
 inverters need.

 For example, there are inverters which communication needs not to be
 interrupted as they can only handle one request at a time and before another
 inverter is allowed to use the communication line the first one has to
 complete the it. Lets call this types "exclusive communication"

 Other inverters are not that strict, called here "non-exclusive". For those it
 is not important if between sending and receiving the answer some other inverter
 sends its own request. The same is true if you have inverters which will send
 their data without the prior need to request it.

 For the implementation, "exlucsive comms" needs to implementation a scheme that
 ensures that only one inverter object can access the communication line at a
 given time. As it is hard to detect for the communication object when the
 inverter
 is done talking, the inverter needs to hint the communication class.
 This can be done by adding a data to the ICommand telling "I need that the line
 a while" and signaling this as long as this is needed. However, to
 differentiate between "exclusive" and "non-exclusive" usage,
 also "I'done" needs to be signaled.

 As an inverter is free to queue more than one command (as comms is a FIFO)
 shared comms needs to keep track of each "atomic commms block",
 additionally a inverter might issue more than one of those
 "atomic comms blocks" in the queue, the shared comms must keep track of this
 individual blocks. It will assign a "ticket" number as soon as the request is
 made and the tickets numbers will be then used to establish a FIFO to also
 consider other customers of the sharedcomms.

 In this atomic blocks, all requests pending can be immediately posted to the real
 comms, as this will handle them accordingly.
 The sharedcomms master however needs to know the completion of an atomic block,
 so it will divert the associated ICommand to call itself as completion handler,
 forward the ICommand to its real target and then continue with queuing the next
 atomic block (if any).

 Non-exclusive inverters have no problems with scheduling the commands, but due
 to the missing synchronization between the actions (what the atomic blocks
 realizes), there are race-conditions when it comes to reading from the comms:
 When two Inverters read at the same time and a telegram for one of them
 arrives,
 bot Inverters will complete there read requests. However, the Inverter which
 is not the recipient will recognize this during parsing and then it will
 re-issue
 the read request. In the meantime some bytes might be already lost, when the
 first Inverter was "faster" and already issued the next read request, which
 would
 completed before the second one issue its subsequent. In this case the 1st
 Inverter would receive the data for the second and then discard this data.
 The second would never this this telegram.
 This can be handled by introducing buffers in the sharedcomms: Every reception
 is buffered and each inverter will receive the buffered data it did not yet see
 on a read request -- even if the data was received when there was no active
  read request at the time.


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

/** SharedComms Atomic-Block Ticket-Token for the SharedCommms
 * Added */
#define ICONN_SHARED_TICKET "ICON_SHARED_TICKET"


#include <assert.h>
#include "Connections/interfaces/IConnect.h"
#include "CSharedConnectionMaster.h"

class CSharedConnection: public IConnect
{
protected:
	friend class IConnectFactory;
	friend class CSharedConnectionSlave;
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

	virtual void Accept(ICommand *callback) {
        assert(concreteSharedConnection);
        concreteSharedConnection->Accept(callback);
	}

	virtual void Noop(ICommand *callback) {
        assert(concreteSharedConnection);
        concreteSharedConnection->Noop(callback);
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

protected:
    IConnect *GetConcreteSharedConnection(void)
    {
        return concreteSharedConnection;
    }

private:

	bool CreateSharedConnectionObject();

	IConnect *concreteSharedConnection;
};

#endif

#endif /* CSHAREDCONNECTION_H_ */
