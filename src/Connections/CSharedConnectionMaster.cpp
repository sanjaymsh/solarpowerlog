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

#include "configuration/Registry.h"
#include "CSharedConnectionMaster.h"
#include "Connections/factories/IConnectFactory.h"
#include "configuration/Registry.h"

enum
{
	CMD_READ
};

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

}

CSharedConnectionMaster::~CSharedConnectionMaster()
{
	if (connection)
		delete connection;

}

void CSharedConnectionMaster::ExecuteCommand(const ICommand *Command)
{

	switch (Command->getCmd())
	{

	case CMD_READ:
	{
		list<ICommand*>::iterator it;
		for (it = readcommands.begin(); it != readcommands.end(); it++) {
			(*it)->mergeData(*Command);
			Registry::GetMainScheduler()->ScheduleWork(*it);
		}
		readcommands.clear();
		read_pending = false;
		break;
	}
	}

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
	if (connection)
		connection->SetupLogger(parentlogger, "");
}

bool CSharedConnectionMaster::Send(const char *tosend, unsigned int len,
		ICommand *callback)
{
	assert (connection);
	return connection->Send(tosend, len, callback);
}

bool CSharedConnectionMaster::Send(const string& tosend, ICommand *callback)
{
	assert (connection);
	return connection->Send(tosend, callback);

}

bool CSharedConnectionMaster::Receive(ICommand *callback)
{
	// Save command for later interpretation.
	readcommands.push_back(callback);

	// If no read is pending, issue a read commmand to the connection,
	// but direct result to our class for later distribution.
	// for this, we copy-construct a ICOmmand and modify it afterwards.
	if (!read_pending) {
		ICommand *cmd = new ICommand(*callback);
		cmd->setTrgt(this);
		cmd->setCmd(CMD_READ);
		read_pending = true;
		return connection->Receive(cmd);
	}

	return true;
}

bool CSharedConnectionMaster::CheckConfig(void)
{
	if (connection)
		return connection->CheckConfig();
	else {
		LOGERROR(logger,"No connection object for shared master comms.");
		return false;
	}
}

bool CSharedConnectionMaster::IsConnected(void)
{
	assert(connection);
	return connection->IsConnected();
}

#endif
