/*
 * CConnectSlave.h
 *
 *  Created on: Sep 13, 2010
 *      Author: tobi
 */

#ifndef CCONNECTSLAVE_H_
#define CCONNECTSLAVE_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_COMMS_SHAREDCONNECTION

#include "interfaces/IConnect.h"
#include "CSharedConnectionMaster.h"

class CSharedConnectionSlave: public IConnect
{
protected:
	friend class CSharedConnection;
	CSharedConnectionSlave(const string & configurationname);
public:
	virtual ~CSharedConnectionSlave();

protected:

	virtual bool Connect(ICommand *callback);

	virtual bool Disconnect(ICommand *callback);

	virtual bool Send(const char *tosend, unsigned int len, ICommand *callback =
			NULL);

	virtual bool Send(const string& tosend, ICommand *callback = NULL);

	virtual bool Receive(ICommand *callback);

	virtual bool CheckConfig(void);

	virtual bool IsConnected(void);

private:
	class CSharedConnection *master;

};

#endif

#endif /* CCONNECTSLAVE_H_ */
