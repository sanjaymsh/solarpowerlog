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

#include "ConnectionTCP.h"
#include "interfaces/IConnect.h"

ConnectionTCP::ConnectionTCP(string configurationname)
: IConnect(configurationname)
{

// TODO Auto-generated constructor stub
}

ConnectionTCP::~ConnectionTCP() {
	// TODO Auto-generated destructor stub
}

bool ConnectionTCP::Connect()
{
}

bool ConnectionTCP::Disconnect()
{
}

bool ConnectionTCP::Send(const char *tosend, int len)
{
}

bool ConnectionTCP::Receive(string & wheretoplace, unsigned int maxsize)
{
}

bool ConnectionTCP::Receive(char *wheretoplace, unsigned int maxsize, int *numreceived)
{
}










