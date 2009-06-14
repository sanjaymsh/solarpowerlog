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

/** \file ConnectionTCP.cpp
 *
 *  Created on: May 21, 2009
 *      Author: tobi
 */

#include "interfaces/IConnect.h"
#include "Connections/CConnectTCP.h"

#include <iostream>
#include <string>

#include "configuration/Registry.h"
#include <libconfig.h++>

using namespace std;

CConnectTCP::CConnectTCP( const string &configurationname ) :
	IConnect(configurationname)
{
	stream = NULL;
	host = NULL;
}

CConnectTCP::~CConnectTCP()
{
	cleanupstream();
}

bool CConnectTCP::Connect()
{

	cleanupstream();

	libconfig::Setting &set = Registry::Instance().GetSettingsForObject(
		ConfigurationPath);
	int port;
	long timeout;

	set.lookupValue("tcpadr", strhost);
	set.lookupValue("tcpport", port);
	if (!set.lookupValue("tcptimeout", timeout))
		timeout = 3000;
	timer = timeout;

	host = new ost::IPV4Host(strhost.c_str());
	try {
		stream = new ost::TCPStream(*host, port, 536, true, timeout);
	} catch (...) {
		cerr << "TCP/IP Connection Error to " << strhost << endl;
		return false;
	}

	stream->setTimeout(timeout);

	return stream->isConnected();
}

bool CConnectTCP::Disconnect()
{
	cleanupstream();
	return true;
}

bool CConnectTCP::Send( const char *tosend, unsigned int len )
{
	// FIXME: This is not really good for binary data, but enough for now.

	if (!IsConnected())
		return false;

	for (int i = 0; i < len; i++)
		(*stream) << tosend[i];
	return (0 == stream->sync());
}

bool CConnectTCP::Send( const string & tosend )
{
	if (!IsConnected())
		return false;

	stream->setTimeout(timer);
	(*stream) << tosend;

	return (0 == stream->sync());
}

bool CConnectTCP::Receive( string & wheretoplace )
{
	if (!IsConnected())
		return false;
	if (!stream->isPending(ost::Socket::pendingInput, 1))
		return false;

	stream->setTimeout(1);
	try {
		(*stream) >> wheretoplace;
		stream->sync();
	} catch (...) {
		return false;
	}
	return true;
}

bool CConnectTCP::IsConnected( void )
{
	if (!stream || !stream->isConnected() || !stream->isActive())
		return false;
	return true;
}

bool CConnectTCP::CheckConfig( void )
{
	string setting;
	bool ret = true;

	libconfig::Setting &set = Registry::Instance().GetSettingsForObject(
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

void CConnectTCP::cleanupstream( void )
{
	if (stream) {
		if (stream->isConnected())
			stream->disconnect();
		delete stream;
		stream = NULL;
	}

	if (host)
		delete host;
}

