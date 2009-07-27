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

/** \fixme COMMENT ME
 *
 *
 * TODO DOCUMENT ME!
 */

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

#include "interfaces/IConnect.h"

using namespace std;

class CConnectTCPAsio : public IConnect
{
public:
	CConnectTCPAsio( const string & configurationname );
	virtual ~CConnectTCPAsio();


	/// Connect to something
	/// NOTE: Needed to be overriden! ALWAYS Open in a NON_BLOCK way, or implement a worker thread
	virtual bool Connect();
	/// Tear down the connection.
	virtual bool Disconnect();

	/// Setup the Logger
	virtual void SetupLogger(const string& parentlogger, const string & ="") {
		IConnect::SetupLogger(parentlogger, "Comms_TCP_ASIO");
	}

	/// Send a array of characters (can be used as binary transport, too)
	virtual bool Send( const char *tosend, unsigned int len );

	virtual bool Send( const string& tosend );
	/// Send a strin Standard implementation only wraps to above Send.
	///
	/// Receive a string. Do now get more than maxxsize (-1 == no limit)
	/// NOTE:
	virtual bool Receive( string &wheretoplace );

	/// Receive a binary stream with maxsize as buffer size and place the actual number received
	/// in the numreceived, which is negative on errors.#
	/// (0 == nothing received)

	virtual bool CheckConfig( void );

	virtual bool IsConnected( void );

private:
	boost::asio::io_service *ioservice;
	boost::asio::ip::tcp::socket *sockt;
	boost::asio::streambuf *data;

	time_t timer;

	void cleanupstream( void );

};

#endif /* CONNECTIONTCP_H_ */
