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
#include <queue>

#include "Connections/interfaces/IConnect.h"
#include "Connections/CAsyncCommand.h"
#include "patterns/ICommandTarget.h"
#include "patterns/ICommand.h"
#include "CSharedConnectionSlave.h"

// Token inserted by this or the slave class to specify individual timeouts.
// At this timestamp, the command can be considered timed-out.
#define SHARED_CONN_TIMEOUTTIMESTAMP "CSharedConnection_Timeout"

#define ICONNECT_TOKEN_PRV_ORIGINALCOMMAND "CSharedConnection_Orig_ICommand"

#define SHARED_CONN_DEFAULTTIMEOUT (3000UL)

class CSharedConnectionMaster : public IConnect , ICommandTarget
{

protected:
    friend class CSharedConnection;
    friend class CSharedConnectionSlave;
    CSharedConnectionMaster(const string & configurationname);

public:
    virtual ~CSharedConnectionMaster();

    void ExecuteCommand(const ICommand *Command);

protected:
    // API Section: Those are from IConnect. They are protected as the
    // call is allowed only through a CSharedConnection object.
    virtual void Connect(ICommand *callback);

    virtual void Disconnect(ICommand *callback);

    virtual void SetupLogger(const string& parentlogger, const string &spezialisation = "");

    virtual void Send(ICommand *cmd);

    virtual void Receive(ICommand *callback);

    virtual void Accept(ICommand *callback);

    virtual bool CanAccept(void);

    virtual void Noop(ICommand *callback);


    virtual bool CheckConfig(void);

    virtual bool IsConnected(void);

    virtual bool AbortAll();

    /// Incoming communication calls from the sharedcomms-slaves.
    void Connect(ICommand *callback, CSharedConnectionSlave *s);

    /// Incoming communication calls from the sharedcomms-slaves.
    void Disconnect(ICommand *callback, CSharedConnectionSlave *s);

    /// Incoming communication calls from the sharedcomms-slaves.
    void Send(ICommand *callback, CSharedConnectionSlave *s);

    /// Incoming communication calls from the sharedcomms-slaves.
    void Receive(ICommand *callback, CSharedConnectionSlave *s);

    void Accept(ICommand *callback, CSharedConnectionSlave *s);

    void Noop(ICommand *callback, CSharedConnectionSlave *s);

    /// Ticket-Service for atomic-block handling
    long GetTicket();

    /** Register a slave for reading result distribution.
     *
     * Adds the slave to the list of recipients
     *
     * \param slave to be subscribed or unsubscribed
     * \param subscribe true to subscribe, false to unsubscribe
     *
     * \note multiple calls to subscribe will cause multiple subscriptions.
     *
     * \note unsubscribing will remove all subscriptions of this slave..
     */
    void SubscribeSlave(CSharedConnectionSlave *slave, bool subscribe= true);

private:

    enum api_id
    {
        API_DISCONNECT, API_CONNECT, API_SEND, API_RECEIVE, API_ACCEPT, API_NOOP
    };

    /** Atomic-Block Handling for incomming API calls
     *
     * Bundled the common handling of slave requests entering the IConnect API.
     *
     * \param cmd the callback
     * \param api_id  API ID
     * \param isatomic if non-null, store the result of the check if it is atomic or not
     *
     * \returns the ICommand to be used for the target IConnect or NULL
     * if no command must be issued at this time.
     * */
    ICommand* HandleAtomicBlock(ICommand *cmd, enum api_id id, bool *isatomic=NULL);

    /** Helper function to dispatch API calls.
     */
    void ICommandDispatcher(ICommand *cmd);

    /** Helper function to handle the interruption of non-atomic receives */
    void _HandleNonAtomicReceiveInterrupts(void);

    IConnect *connection;

    list<ICommand*> readcommands;

    boost::mutex mutex;
    long ticket_cnt;
    long active_ticket;

    /// Proxy to handle the direct calls from the inverter. (code reuse)
    CSharedConnectionSlave *ownslave;

    /// list of "free" (non-atomic) commands pending.
    std::queue<ICommand*> non_atomic_icommands_pending;

    /** container to store all (ticketed) atomic blocks commands that are
     * pending
     * map-key is the ticket id, the embedded queue the commands associated
     * with it.
     */
    std::map<long, std::queue<ICommand*> > atomic_icommands_pending;

    /** The ICommand that will end this block.*/
    ICommand *last_atomic_cmd;

    bool non_atomic_mode;

    /// List of "listening" slaves.
    std::list<CSharedConnectionSlave *> _reading_slaves;

    /// When is the current receive scheduled to timeout?
    boost::posix_time::ptime readtimeout;

    bool _nam_interrupted;

};

#endif

#endif /* CSHAREDCONNECTIONMASTER_H_ */
