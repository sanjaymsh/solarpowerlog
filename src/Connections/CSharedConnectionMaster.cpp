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
#include "Connections/factories/IConnectFactory.h"


// THOUGHTS:
/*
 * - the master talks to the real connection class
 * - (the first version) will just do a FIFO in comms, so getting the
 *   requests from the inverter, adding them to an internal list and
 *   then propagate them to the comms object, acting as a negotiater.
 * - it cannot just add them to the targets comms queue, as receive events
 *   might impose race conditions so that a answer would be routed to a wrong
 *   destiation or a message would be concentreated so that a inverter gets its
 *   own and a foreign one (or in the other sequence)
 *   Another issue is that there might be inverters that do not need an question
 *   to answer, but keep talking without request. The Connection class could and
 *   should not have logic to sort them apart.
 *   This is solved that all inverters currently waiting for receiption will
 *   get the answer and as they have the knowledge about the telegramms, they
 *   easily can sort out the other telegrams.
 *
 * */


CSharedConnectionMaster::CSharedConnectionMaster(
		const string & configurationname) :
	IConnect(configurationname)
{
	// Get real configuration path to extract target comms config.
	string commsconfig = configurationname + ".CommsConfig";
	connection = IConnectFactory::Factory(commsconfig);

	sem_init(&cmdsemaphore, 0, 0);
	StartWorkerThread();

}

CSharedConnectionMaster::~CSharedConnectionMaster()
{
	if (connection) delete connection;

}

bool CSharedConnectionMaster::Connect(ICommand *callback)
{
	assert(connection);
	return connection->Connect(callback);

}

bool CSharedConnectionMaster::Disconnect(ICommand *callback)
{

}

void CSharedConnectionMaster::SetupLogger(const string& parentlogger,
		const string &)
{
	if (connection) connection->SetupLogger(parentlogger,"");
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

void CSharedConnectionMaster::_main(void)
{

}


#endif
