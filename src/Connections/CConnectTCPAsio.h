/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (ConnectionTCP.h), the license terms are:

 You can redistribute it and/or  modify it under the terms of the GNU Lesser
 General Public License (LGPL) as published by the Free Software Foundation;
 either version 3 of the License, or (at your option) any later version.

 This programm is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/** \file ConnectionTCPAsio.h
 *
 *  Created on: May 21, 2009
 *      Author: tobi
 */

#ifndef CONNECTIONTCPASIO_H_
#define CONNECTIONTCPASIO_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

#include "interfaces/IConnect.h"
#include "interfaces/CWorkScheduler.h"
#include "configuration/Registry.h"
#include "patterns/ICommand.h"

#include <semaphore.h>

/// Default timeout for all operations, if not configured
#define TCP_ASIO_DEFAULT_TIMEOUT (3000UL)

using namespace std;

class asyncCommand
{
public:
	enum Commando
	{
		DISCONNECT, /// Tear down a connection
		CONNECT, /// Connect
		SEND, /// Send data
		RECEIVE,
	/// Try to receive data
	};

	asyncCommand( enum Commando c, ICommand *callback, sem_t *sem = NULL )
	{
		this->c = c;
		if (!callback) {
			this->callback = new ICommand(NULL, NULL);
			private_icommand = true;
		} else {
			this->callback = callback;
			private_icommand = false;
		}

		this->sem = sem;
	}

	~asyncCommand()
	{
		if (private_icommand)
			delete callback;
	}

	void SetSemaphore( sem_t *sem )
	{
		this->sem = sem;
	}

	/** Handle this jobs completion by notifying the sender
	 *
	 */
	void HandleCompletion( void )
	{
		if (!private_icommand) {
			Registry::GetMainScheduler()->ScheduleWork(callback);
		} else {
			sem_post(sem);
		}
	}

	/** Is the asyncCommnd really async, or was it only pretended?
	 *
	 * As syncronous operations are also dispatched asynchronous,
	 * but we need a ICommand-object for this, we need the information
	 * if it is sync or not to decide when to delete the object.
	 * */
	bool IsAsynchronous()
	{
		return !private_icommand;
	}

	enum Commando c; ///< what to do

	/** callback for completion handling
	 * In this ICommand, the comand data is stored, results and data...
	 * This ICommand is privately created, if private_icommand is true,
	 * and will not be executed if so, but can be still be used for
	 * storage  */
	ICommand *callback;
private:
	sem_t *sem; ///< if not-null, use this semaphore to notify completion.
	/// \note: it is context specific to check if it worked out or not.
	/// The semaphore is intended to be used for the "synchronous fallback"

	bool private_icommand;

};

/** This class implements a method to connect to TCP/IP via the boost ASIO
 * library.
 *
 * For the interface documentation, please see IConnect.
 *
 *
 */
class CConnectTCPAsio: public IConnect
{
protected:

public:
	CConnectTCPAsio( const string & configurationname );
	virtual ~CConnectTCPAsio();

	virtual bool Connect( ICommand *callback );

	virtual bool Disconnect( ICommand *callback );

	virtual void SetupLogger( const string& parentlogger, const string & = "" )
	{
		IConnect::SetupLogger(parentlogger, "Comms_TCP_ASIO");
	}

	virtual bool Send( const char *tosend, unsigned int len,
			ICommand *callback = NULL );

	virtual bool Send( const string& tosend, ICommand *callback = NULL );

	virtual bool Receive( string &wheretoplace, ICommand *callback );

	virtual bool CheckConfig( void );

	virtual bool IsConnected( void );

private:
	boost::asio::io_service *ioservice;
	boost::asio::ip::tcp::socket *sockt;
	boost::asio::streambuf *data;

	time_t timer;

	// async patch
	virtual void _main( void );

	/** push some new work to the worker thread.
	 *  \note: If the work can be handled synchronously more efficent, the
	 *  work might be executed right away.
	 *
	 * \param cmd to be executed. Will take ownership of object and destroy
	 * it after use. (in other words: will be deleted. But only the struct,
	 * not containing objects!)
	 *
	 * \returns false if work could not be pushed or true if it worked out.
	 */
	bool PushWork( asyncCommand *cmd );

	/// cancel all current work.
	//void CancelWork( void );

	/** Handle "Connect-Command"
	 *
	 * Connects to the configured target.
	 *
	 * \returns true, if job can be removed from queue, false, if it needs to
	 * be handled again
	 *
	 *
	 * */
	bool HandleConnect( asyncCommand *cmd );

	/** Handle the disconnect command.
	 *
	 * \returns true, if job can be removed from queue, false, if it needs to
	 * be handled again
	 */

	bool HandleDisConnect( asyncCommand *cmd );

	bool HandleReceive( asyncCommand *cmd );

	bool HandleSend( asyncCommand *cmd );

	list<asyncCommand*> cmds;
	sem_t cmdsemaphore;

};

#endif /* CONNECTIONTCP_H_ */
