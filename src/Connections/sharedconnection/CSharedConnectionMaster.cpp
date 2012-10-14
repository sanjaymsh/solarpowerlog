/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

 Copyright (C) 2010-2012 Tobias Frost

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ----------------------------------------------------------------------------
 */

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

#include <boost/date_time.hpp>
#include "configuration/CConfigHelper.h"
#include "interfaces/CMutexHelper.h"
#include "CSharedConnection.h"
#include "patterns/ICommand.h"

#define STATUS_CONNECTED (1<<0)

#define ICONN_SHAREDCOMMS_APIID "ICONN_CSC_APIID"

enum
{
    CMD_HANDLEENDOFBLOCK = BasicCommands::CMD_USER_MIN
};

CSharedConnectionMaster::CSharedConnectionMaster(
    const string & configurationname) :
    IConnect(configurationname)
{
    connection = NULL;
    last_atomic_cmd = NULL;
    ticket_cnt = active_ticket = 0;
    ownslave = new CSharedConnectionSlave(configurationname);
    ownslave->setMaster(this);

}

CSharedConnectionMaster::~CSharedConnectionMaster()
{
    if (connection) delete connection;
    delete ownslave;
}

void CSharedConnectionMaster::SetupLogger(const string& parentlogger,
    const string &)
{
    IConnect::SetupLogger(parentlogger, "");
    if (connection) connection->SetupLogger(parentlogger, "");
    ownslave->SetupLogger(parentlogger);
}


void CSharedConnectionMaster::ExecuteCommand(const ICommand *Command)
{
    // handling end of atomic blocks and issuing pending commands.
    switch (Command->getCmd()) {
        case CMD_HANDLEENDOFBLOCK: {
            // ok, we've got signalled that the last atomic block command has
            // just been finished.
            // redirect the answer and clean up our stats.
            CMutexAutoLock m(&mutex); // mutex is needed....
            LOGDEBUG(logger, "Ending Ticket " << active_ticket );
            assert(last_atomic_cmd);
            last_atomic_cmd->mergeData(*Command);
            Registry::GetMainScheduler()->ScheduleWork(last_atomic_cmd);
            last_atomic_cmd = NULL;
            active_ticket = 0;

            // answer redirected, now lets check for pending work.
            // first the "non-atomic-ones"
            // they can just be queued to the comms object

            LOGDEBUG(logger,"non-atomic-backlog:" << non_atomic_icommands_pending.size());

            while (non_atomic_icommands_pending.size()) {
                ICommand *cmd = non_atomic_icommands_pending.front();
                non_atomic_icommands_pending.pop();
                ICommandDispatcher(cmd);
             }

            LOGDEBUG(logger,"atomic-backlog (blocks):" << atomic_icommands_pending.size());

            // after giving the non-atomic priority, lets resume atomic operation.
            // we can also queue the next atomic block completly, but we need
            // again to catch the cmd completing the block.
            // as std::maps are ordered, we can just "pop" the first item#
            if (atomic_icommands_pending.size()) {
                ICommand *cmd;
                std::map<long, std::queue<ICommand*> >::iterator it =
                    atomic_icommands_pending.begin();
                active_ticket = it->first;
                std::queue<ICommand *> *q = &(it->second); // Convenience only :)
                while (q->size() > 1) {
                    cmd = q->front();
                    q->pop();
                    ICommandDispatcher(cmd);
                }


                // now we have one left in the queue, but this could be one
                // finishing the queue (then we need to intercept the result)
                // or it just one block in the middle of the atomic block
                // in this case we can just submit the command to the comms.
                assert(q->size() == 1);
                cmd = q->front();
                bool end_of_block = !boost::any_cast<bool>(
                    cmd->findData(ICONN_ATOMIC_COMMS));

                if (end_of_block) {
                    ICommand *newcmd = new ICommand(CMD_HANDLEENDOFBLOCK, this);
                    newcmd->mergeData(*cmd);
                    last_atomic_cmd = cmd;
                    cmd->RemoveData(); // free up some memory, data not needed.
                } else {
                    ICommandDispatcher(cmd);
                }

                LOGDEBUG(logger, "New Ticket " << active_ticket );

                // we do not need to keep this map item anymore.
                atomic_icommands_pending.erase(it);
            }
            break;
        }
    }
#warning fehlt: -> Unterstützung der atomic blocks in den Invertern
#warning -> callbacks aller commands dürfen nicht mehr NULL sein (diverse Comms updaten)
#warning -> noch nicht gedebuggt.
#warning -> (Mindest-Unterstützung) CCommsSharedSlave von non-atomic blöcken (auch hier kann connect/disconect ein problem sein)
#warning -> Entscheidung wessen Connect/Disconnect weitergereicht wird (alle? nur master? Algo?)
}

void CSharedConnectionMaster::Connect(ICommand *callback)
{
    ownslave->Connect(callback);
}

void CSharedConnectionMaster::Disconnect(ICommand *callback)
{
    ownslave->Disconnect(callback);
}

void CSharedConnectionMaster::Send(ICommand *callback)
{
    ownslave->Send(callback);
}

void CSharedConnectionMaster::Receive(ICommand *callback)
{
    ownslave->Receive(callback);
}

void CSharedConnectionMaster::Accept(ICommand* callback)
{
    ownslave->Accept(callback);
}

void CSharedConnectionMaster::Noop(ICommand* callback)
{
    ownslave->Noop(callback);
}

bool CSharedConnectionMaster::CheckConfig(void)
{
    // Get real configuration path to extract target comms config.
    string commsconfig = this->ConfigurationPath + ".realcomms";
    string s;
    CConfigHelper h(commsconfig);

    if (!h.GetConfig("comms", s)) {
        LOGERROR(logger, "realcomms section: comms missing");
        return false;
    }

    connection = IConnectFactory::Factory(commsconfig);

    // connection always valid -- the factory returns a dummy
    // object if it does not know the comms.
    if (connection) {
        connection->SetupLogger(logger.getLoggername(),"realcomms");
        return connection->CheckConfig();
    }
    LOGFATAL(logger,"Could not create real communication object");
    return false;
}

bool CSharedConnectionMaster::IsConnected(void)
{
    assert(connection);
    return connection->IsConnected();
}

bool CSharedConnectionMaster::AbortAll()
{
    assert(connection);
    return connection->AbortAll();
}

long CSharedConnectionMaster::GetTicket()
{
    CMutexAutoLock lock(&mutex);
    ++ticket_cnt;
    // Overflow protection -- the zero is reserved for "no ticket"
    if (0 == ticket_cnt) ticket_cnt++;
    LOGDEBUG(logger," New ticket requested:" << ticket_cnt);
    return ticket_cnt;
}

void CSharedConnectionMaster::Connect(ICommand* callback,
    CSharedConnectionSlave* s)
{
    assert(callback);
    assert(s);
    callback = HandleAtomicBlock(callback, API_CONNECT);
    if (callback) connection->Connect(callback);
}

void CSharedConnectionMaster::Disconnect(ICommand* callback,
    CSharedConnectionSlave* s)
{
    assert(callback);
    assert(s);
    callback = HandleAtomicBlock(callback, API_DISCONNECT);
    if (callback) connection->Disconnect(callback);
}

void CSharedConnectionMaster::Send(ICommand* callback,
    CSharedConnectionSlave* s)
{
    assert(callback);
    assert(s);
    callback = HandleAtomicBlock(callback, API_SEND);

    std::string st = boost::any_cast<std::string>(callback->findData(ICONN_TOKEN_SEND_STRING));
    LOGDEBUG(logger,"to-send: "<< st);

    if (callback) connection->Send(callback);
}

void CSharedConnectionMaster::Receive(ICommand* callback,
    CSharedConnectionSlave* s)
{
    assert(callback);
    assert(s);
    callback = HandleAtomicBlock(callback, API_RECEIVE);
    if (callback) connection->Receive(callback);
}

void CSharedConnectionMaster::Noop(ICommand* callback,
    CSharedConnectionSlave* s)
{
    assert(callback);
    assert(s);
    callback = HandleAtomicBlock(callback, API_NOOP);
    if (callback) connection->Noop(callback);

}

void CSharedConnectionMaster::Accept(ICommand* callback,
    CSharedConnectionSlave* s)
{
#warning handle accept here as connect!
    assert(callback);
    assert(s);
    callback = HandleAtomicBlock(callback, API_ACCEPT);
    if (callback) connection->Accept(callback);
}

ICommand* CSharedConnectionMaster::HandleAtomicBlock(ICommand* cmd,
    enum api_id id)
{
    long ticket = 0;
    try {
        ticket = boost::any_cast<long>(cmd->findData(ICONN_SHARED_TICKET));
    } catch (...) {
    }

    bool end_of_block = false;
    try {
        end_of_block = !boost::any_cast<bool>(
            cmd->findData(ICONN_ATOMIC_COMMS));
    } catch (...) {
        // assert if we could not retrieve this in a signaled atomic block.
        assert(!ticket);
    }

    CMutexAutoLock m(&mutex);
    LOGDEBUG(logger,"Ticket for this command is: "<< ticket << " (current ticket is "
        << active_ticket << ")");

    // check if this received work is part of an atomic block
    if (ticket) {
        // -> yes: check if the active one
        // check if we currently got an atomic-block
        if (!active_ticket) {
            // nope, this will start it.
            active_ticket = ticket;
            LOGDEBUG(logger, "New ticket: "<< ticket);
        }

        if (active_ticket == ticket) {
            //-> yes: check if the current block ends the atomic block
            if (end_of_block) {
                //-> yes: this ends the atomic block.
                // So save Icommand and create a new one to redirect to own
                // for completion handling in own ICommandTrgt. Send new one to comms.
                LOGDEBUG(logger, "Ticket: "<< ticket << " ends soon");
                ICommand *newcmd = new ICommand(CMD_HANDLEENDOFBLOCK, this);
                newcmd->mergeData(*cmd);
                assert(!last_atomic_cmd);
                last_atomic_cmd = cmd;
                cmd->RemoveData(); // free up some memory, this copy wont need
                // the data.
                return newcmd;
            } else {
                //-> no: just send to comms directly,
                // the communication will handle it with its fifo.
                LOGDEBUG(logger, "Ticket: "<< ticket << " continues");
                return cmd;
            }
        } else {
            // -> no, not part of active atomic block, queue for later.
            // (as currently the interface is occupied)
            cmd->addData(ICONN_SHAREDCOMMS_APIID, id);
            // check if this sets a completly new atomic block
            if (!atomic_icommands_pending.count(ticket)) {
                LOGDEBUG(logger, "Ticket: "<< ticket << " new backlog entry");

                // Yes, we need a new map-entry.
                std::pair<long, std::queue<ICommand*> > p;
                p.first = ticket;
                p.second.push(cmd);
                atomic_icommands_pending.insert(p);
            } else {
                // No, just append to queue.
                LOGDEBUG(logger, "Ticket: "<< ticket << " new backlog entry to queue");
                std::map<long, std::queue<ICommand*> >::iterator it =
                    atomic_icommands_pending.find(ticket);
                it->second.push(cmd);
            }
            return NULL;
        }
    } else {
        // -> no, not atomic.  check if currently an atomic block is locking the comms
        if (active_ticket) {
            // -> yes, queue for later
            cmd->addData(ICONN_SHAREDCOMMS_APIID, id);
            non_atomic_icommands_pending.push(cmd);
            return NULL;
        } else {
            // -> no, send directly to comms
            return cmd;
        }
    }
}

void CSharedConnectionMaster::ICommandDispatcher(ICommand* cmd)
{
    api_id api = boost::any_cast<api_id>(
        cmd->findData(ICONN_SHAREDCOMMS_APIID));

    // forward the api calls to the real comms object.
    switch (api) {
        case API_CONNECT:
            connection->Connect(cmd);
        break;

        case API_DISCONNECT:
            connection->Disconnect(cmd);
        break;

        case API_RECEIVE:
            connection->Receive(cmd);

        break;

        case API_SEND:
            connection->Send(cmd);
        break;

        case API_ACCEPT:
            connection->Accept(cmd);
        break;

        case API_NOOP:
            connection->Noop(cmd);
        break;

        default:
            assert(0);
        break;
    }
}


#endif
