/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (ConnectionTCP.cpp), the license terms are:

 You can redistribute it and/or modify it under the terms of the GNU
 General Public License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This programm is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/** \file ConnectionTCPBoost.cpp
 *
 *  Created on: May 21, 2009
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#include "interfaces/IConnect.h"
#include "Connections/CConnectTCPAsio.h"

#include <iostream>
#include <string>

#include "configuration/Registry.h"
#include "configuration/CConfigHelper.h"

#include <boost/asio/write.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

using namespace std;
using namespace boost::asio;
using namespace boost;
using namespace libconfig;

CConnectTCPAsio::CConnectTCPAsio( const string &configurationname ) :
	IConnect(configurationname)
{
	// Generate our own asio ioservoce
	// TODO check if one central would do that too...
	ioservice = new io_service;
	sockt = new ip::tcp::socket(*ioservice);
}

CConnectTCPAsio::~CConnectTCPAsio()
{
	cleanupstream();

	if (sockt)
		delete sockt;
	if (ioservice)
		delete ioservice;

}

bool CConnectTCPAsio::Connect()
{
	string strhost, port;
	unsigned long timeout;

	CConfigHelper cfghelper(ConfigurationPath);
	cfghelper.GetConfig("tcpadr", strhost);
	cfghelper.GetConfig("tcpport", port);
	cfghelper.GetConfig("tcptimeout", timeout, 3000UL);

	cleanupstream();
	boost::system::error_code ec;

	ip::tcp::resolver resolver(*ioservice);
	ip::tcp::resolver::query query(strhost.c_str(), port);
	ip::tcp::resolver::iterator iter = resolver.resolve(query);
	ip::tcp::resolver::iterator end; // End marker.

	// TODO Change to async connect.
	while (iter != end) {
		ip::tcp::endpoint endpoint = *iter++;
		LOG_DEBUG(logger, "Connecting to " << endpoint );
		sockt->connect(endpoint, ec);
		if (!ec)
			break;
	}

	cfghelper.GetConfig("name", strhost);

	if (ec) {

		LOG_DEBUG(logger, "Connection to " << strhost << " failed" );
		return false;

	}

	LOG_DEBUG(logger, "Connected to " << strhost );
	return true;

}

bool CConnectTCPAsio::Disconnect()
{
	cleanupstream();
	return true;
}

bool CConnectTCPAsio::Send( const char *tosend, unsigned int len )
{
	size_t written;
	written = asio::write(*sockt, asio::buffer(tosend, len));
	return (written == len);
}

bool CConnectTCPAsio::Send( const string & tosend )
{
	return Send(tosend.c_str(), tosend.length());
}

bool CConnectTCPAsio::Receive( string & wheretoplace )
{
	size_t avail = sockt->available();
	size_t recvd;
	if (!avail)
		return false;

	char buf[avail + 1];
	buf[avail] = 0;

	recvd = sockt->read_some(asio::buffer(buf, avail));
	if (!recvd)
		return false;

	wheretoplace.assign(buf, recvd);
	return true;
}

bool CConnectTCPAsio::IsConnected( void )
{
	return sockt->is_open();
}

bool CConnectTCPAsio::CheckConfig( void )
{
	string setting;
	bool fail = false;

	CConfigHelper cfghelper(ConfigurationPath);
	fail |= !cfghelper.CheckConfig("tcpadr",
		libconfig::Setting::TypeString);
	fail |= !cfghelper.CheckConfig("tcpport",
		libconfig::Setting::TypeString);
	fail |= !cfghelper.CheckConfig("tcptimeout",
		libconfig::Setting::TypeInt, false);

	return !fail;
}

void CConnectTCPAsio::cleanupstream( void )
{
	if (sockt->is_open())
		sockt->close();
}

