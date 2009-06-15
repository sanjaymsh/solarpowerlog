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

#include "interfaces/IConnect.h"
#include "Connections/CConnectTCPAsio.h"

#include <iostream>
#include <string>

#include "configuration/Registry.h"
#include <libconfig.h++>

#include <boost/asio.hpp>

using namespace std;

using namespace boost::asio;
using namespace boost;


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

	cleanupstream();

	boost::system::error_code ec;

	libconfig::Setting & set = Registry::Instance().GetSettingsForObject(
		ConfigurationPath);

	string strhost, port;
	long timeout;

	set.lookupValue("tcpadr", strhost);
	set.lookupValue("tcpport", port);
	if (!set.lookupValue("tcptimeout", timeout))
		timeout = 3000;

	ip::tcp::resolver resolver(*ioservice);
	ip::tcp::resolver::query query(strhost.c_str(), "12345");
	ip::tcp::resolver::iterator iter = resolver.resolve(query);
	ip::tcp::resolver::iterator end; // End marker.

	// TODO Change to async connect.
	while (iter != end) {
		ip::tcp::endpoint endpoint = *iter++;
		std::cout << "Connecting to ..." << endpoint << std::flush;
		sockt->connect(endpoint, ec);
		if (!ec)
			break;
	}

	if (ec) {
		cerr << " failed" << endl;
		return false;

	}

	cout << " succesful " << endl;
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
	bool ret = true;

	libconfig::Setting & set = Registry::Instance().GetSettingsForObject(
		ConfigurationPath);

	setting = "tcpadr";
	if (!set.exists(setting) || !set.getType()
		== libconfig::Setting::TypeString) {
		cerr << "Setting " << setting << " in " << ConfigurationPath
			<< " missing or of wrong type (wanted a string)"
			<< endl;
		ret = false;
	}

	setting = "tcpport";
	if (!set.exists(setting) || !set.getType()
		== libconfig::Setting::TypeInt) {
		cerr << "Setting " << setting << " in " << ConfigurationPath
			<< " missing or of wrong type (wanted a integer)"
			<< endl;
		ret = false;
	}

	return ret;
}

void CConnectTCPAsio::cleanupstream( void )
{
	if (sockt->is_open())
		sockt->close();
}

