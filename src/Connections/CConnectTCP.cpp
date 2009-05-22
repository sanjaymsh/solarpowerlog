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

#include "iostream"

using namespace std;

CConnectTCP::CConnectTCP(const string &configurationname)
: IConnect(configurationname)
{

// TODO Auto-generated constructor stub
}

CConnectTCP::~CConnectTCP() {
	// TODO Auto-generated destructor stub
}

bool CConnectTCP::Connect()
{
	// TODO Implement me
}

bool CConnectTCP::Disconnect()
{
	// TODO Implement me
}

bool CConnectTCP::Send(const char *tosend, int len)
{
	// TODO Implement me
}

bool CConnectTCP::Receive(string & wheretoplace, unsigned int maxsize)
{
	// TODO Implement me
}

bool CConnectTCP::Receive(char *wheretoplace, unsigned int maxsize, int *numreceived)
{
	// TODO Implement me
}

bool CConnectTCP::CheckConfig(void)
{
	// TODO Implement me
	cerr << __FUNCTION__ << ": Not implemented yet" << endl;
	return false;
}












