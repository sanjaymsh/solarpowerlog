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

class CSharedConnectionMaster: public IConnect
{

protected:
	friend class CSharedConnection;
	CSharedConnectionMaster(const string & configurationname);

public:
	virtual ~CSharedConnectionMaster();

protected:

	virtual bool Connect(ICommand *callback);

	virtual bool Disconnect(ICommand *callback);

	virtual void SetupLogger(const string& parentlogger, const string & = "");

	virtual bool Send(const char *tosend, unsigned int len, ICommand *callback =
			NULL);

	virtual bool Send(const string& tosend, ICommand *callback = NULL);

	virtual bool Receive(ICommand *callback);

	virtual bool CheckConfig(void);

	virtual bool IsConnected(void);

private:
	virtual void _main( void );

	IConnect *connection;

	list<CAsyncCommand*> cmds;
	sem_t cmdsemaphore;


};

#endif

#endif /* CSHAREDCONNECTIONMASTER_H_ */
