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
/** \file CConnectSlave.h
 *
 * This is the slave object for the Shared Communication.Please see the
 * CSharedConnection class for documentation...
 *
 * (In few words, this will will proxy requests over to amaster object
 * which then will do the comms)
 *
 *  Created on: Sep 13, 2010
 *      Author: coldtobi
 */

#ifndef CCONNECTSLAVE_H_
#define CCONNECTSLAVE_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_COMMS_SHAREDCONNECTION

#include "Connections/interfaces/IConnect.h"
#include "patterns/ICommandTarget.h"

class CSharedConnectionMaster;

class CSharedConnectionSlave: public IConnect, protected ICommandTarget
{
protected:
	friend class CSharedConnection;
    friend class CSharedConnectionMaster;
	CSharedConnectionSlave(const string & configurationname);
public:
	virtual ~CSharedConnectionSlave();

	virtual void ExecuteCommand(const ICommand *cmd);

protected:
    void setMaster(CSharedConnectionMaster* master)
    {
        LOGDEBUG(logger,"this:" << this << " master:"<<master);
        this->master = master;
    }

    enum Commands
    {
        CMD_HANDLETIMEOUTS = BasicCommands::CMD_USER_MIN,
        CMD_HANDLEREAD
    };


protected:

	virtual void Connect(ICommand *callback);

	virtual void Disconnect(ICommand *callback);

	virtual void Send(ICommand *cmd);

	virtual void Receive(ICommand *callback);

	virtual void Accept(ICommand *callback);

	virtual bool CheckConfig(void);

	virtual bool IsConnected(void);

    virtual bool AbortAll();

    virtual void Noop(ICommand *cmd);

    virtual bool CanAccept(void);

    /** Handles the common tasks regarding the ticket system to handler "atomic
     * blocks"
     *
     * It will
     * - examine if a new ticket is needed
     * - examine if the ticket is about to be closed
     * - add the ticket to ICommand for the SharedMaster
     *
     * \param callback callback to be fixed
     *
     * \returns true if the command requests atomic operation.
     * */
    bool HandleTickets(ICommand *callback);

private:
	CSharedConnectionMaster *master;

	long current_ticket;

	bool slave_registered;

    std::list<ICommand *> pending_reads;

    boost::mutex mutex;

    /// Buffer for non-atomic reads while no read is pending.
    /// Will be reset by Connect and Disconnect.
    std::string read_buffer;
};

#endif

#endif /* CCONNECTSLAVE_H_ */
