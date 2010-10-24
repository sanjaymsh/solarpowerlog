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

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/** \file CConnectSerialAsio.h
 *
 * This interface class abstracts the Comms-Interface (IConnect) to the
 * boost::asio class
 *
 * Boost::asio can act either as a serial class (like RS233, Rs485)
 * or as TCP class. so copying would mean lots of duplicated code
 *
 *  Created on: May 16, 2010
 *      Author: tobi
 */

#ifndef CCONNCECTSERIALASIO_H_
#define CCONNCECTSERIALASIO_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_COMMS_ASIOSERIAL

#include <boost/asio/serial_port.hpp>
#include <boost/asio/streambuf.hpp>
#include <semaphore.h>

#include "interfaces/IConnect.h"
#include "interfaces/CWorkScheduler.h"
#include "configuration/Registry.h"
#include "patterns/ICommand.h"
#include "Connections/CAsyncCommand.h"

/// Default timeout for all operations, if not configured
#define TCP_ASIO_DEFAULT_TIMEOUT (3000UL)
#define TCP_ASIO_DEFAULT_INTERBYTETIMEOUT (50UL)

using namespace std;

/** This class implements a method to connect to TCP/IP via the boost ASIO
 * library.
 *
 * For the interface documentation, please see IConnect.
 *
 *
 */
class CConnectSerialAsio: public IConnect
{
protected:
	friend class IConnectFactory;
	CConnectSerialAsio(const string & configurationname);

public:
	virtual ~CConnectSerialAsio();

	virtual bool Connect(ICommand *callback);

	virtual bool Disconnect(ICommand *callback);

	virtual void SetupLogger(const string& parentlogger, const string & = "")
	{
		IConnect::SetupLogger(parentlogger, "Comms_TCP_ASIO");
	}

	virtual bool Send(const char *tosend, unsigned int len, ICommand *callback =
			NULL);

	virtual bool Send(const string& tosend, ICommand *callback = NULL);

	virtual bool Receive(ICommand *callback);

	virtual bool CheckConfig(void);

	virtual bool IsConnected(void);

private:
	boost::asio::io_service *ioservice;
	boost::asio::serial_port *port;
	boost::asio::streambuf *data;

	char characterlen;
	boost::asio::serial_port_base::parity parity;
	boost::asio::serial_port_base::stop_bits stopbits;
	boost::asio::serial_port_base::flow_control flowctrl;
	unsigned int baudrate;

	time_t timer;

	// async patch
	virtual void _main(void);

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
	bool PushWork(CAsyncCommand *cmd);

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
	bool HandleConnect(CAsyncCommand *cmd);

	/** Handle the disconnect command.
	 *
	 * \returns true, if job can be removed from queue, false, if it needs to
	 * be handled again
	 */

	bool HandleDisConnect(CAsyncCommand *cmd);

	bool HandleReceive(CAsyncCommand *cmd);

	bool HandleSend(CAsyncCommand *cmd);

	list<CAsyncCommand*> cmds;
	sem_t cmdsemaphore;

};

#endif /* HAVE_COMMS_ASIOSERIAL */

#endif /* CONNECTIONTCP_H_ */
