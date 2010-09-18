/**
 * \file CSharedConnection.h
 *
 * Sharing a connection between 2 or more inverters.
 *
 * This class "manages" the communication between two or more ineverters,
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
 * The receiving inverter has to take care about filtering out answers not, as
 * there are inverters which do not employ a question-answer scheme, but just
 * answering.
 *
 * So, all inverters with pending receive commands will be informed...
 *
 *
 *
 *  Created on: Sep 12, 2010
 *      Author: tobi
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

	virtual bool Send(const char *tosend, unsigned int len, ICommand *callback =
			NULL)
	{
		assert(concreteSharedConnection);
		return concreteSharedConnection->Send(tosend, len, callback);
	}

	virtual bool Send(const string& tosend, ICommand *callback = NULL)
	{
		assert(concreteSharedConnection);
		return concreteSharedConnection->Send(tosend, callback);
	}

	virtual bool Receive(string &wheretoplace, ICommand *callback)
	{
		assert(concreteSharedConnection);
		return concreteSharedConnection->Receive(wheretoplace, callback);
	}

	virtual bool CheckConfig(void);

	virtual bool IsConnected(void) {
		assert(concreteSharedConnection);
		return concreteSharedConnection->IsConnected();
	}

private:

	bool CreateSharedConnectionObject();

	IConnect *concreteSharedConnection;

};

#endif

#endif /* CSHAREDCONNECTION_H_ */
