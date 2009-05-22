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

/** \file ConnectionTCP.h
 *
 *  Created on: May 21, 2009
 *      Author: tobi
 */

#ifndef CONNECTIONTCP_H_
#define CONNECTIONTCP_H_

/** \fixme COMMENT ME
 *
 *
 * TODO DOCUMENT ME!
 */
#include "IConnect.h"

class CConnectTCP: public IConnect {
public:
	CConnectTCP(const string & configurationname);
	virtual ~CConnectTCP();

	/// Connect to something
	/// NOTE: Needed to be overriden! ALWAYS Open in a NON_BLOCK way, or implement a worker thread
	virtual bool Connect();
	/// Tear down the connection.
	virtual bool Disconnect();


	/// Send a array of characters (can be used as binary transport, too)
	virtual bool Send(const char *tosend, int len);
	/// Send a strin Standard implementation only wraps to above Send.
	///
	/// Receive a string. Do now get more than maxxsize (-1 == no limit)
	/// NOTE:
	virtual bool Receive(string &wheretoplace, unsigned int maxsize = -1);

	/// Receive a binary stream with maxsize as buffer size and place the actual number received
	/// in the numreceived, which is negative on errors.#
	/// (0 == nothing received)
	virtual bool Receive(char *wheretoplace, unsigned int maxsize, int *numreceived);

	virtual bool CheckConfig(void) ;

};

#endif /* CONNECTIONTCP_H_ */
