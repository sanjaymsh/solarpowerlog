/* ----------------------------------------------------------------------------
   solarpowerlog
   Copyright (C) 2009  Tobias Frost

   This file is part of solarpowerlog.

   Solarpowerlog is free software; However, it is dual-licenced
   as described in the file "COPYING".

   For this file (IConnect.h), the license terms are:

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


/** \file IConnect.h
 *
 *  Created on: May 16, 2009
 *      Author: tobi
 */

#ifndef ICONNECT_H_
#define ICONNECT_H_

#include <string>

using namespace std;


/** \fixme COMMENT ME
 *
 *
 * TODO DOCUMENT ME!
 */
class IConnect {
public:
	IConnect(const string &configurationname);
	virtual ~IConnect();

	/// Connect to something
	/// NOTE: Needed to be overriden! ALWAYS Open in a NON_BLOCK way, or implement a worker thread
	virtual bool Connect() = 0;
	/// Tear down the connection.
	virtual bool Disconnect() = 0;


	/// Send a array of characters (can be used as binary transport, too)
	virtual bool Send(const char *tosend, int len) = 0;
	/// Send a strin Standard implementation only wraps to above Send.
	///
	/// Override if needed.g (usually comms is "human readable")
	virtual bool Send(const string& tosend) {
		// Standard implementation only wraps to above Send.
		// Override if needed.
		return Send(tosend.c_str(),tosend.length());
	}

	/// Receive a string. Do now get more than maxxsize (-1 == no limit)
	/// NOTE:
	virtual bool Receive(string &wheretoplace) = 0;

	virtual bool CheckConfig(void) = 0 ;

protected:
	string ConfigurationPath;

};

#endif /* ICONNECT_H_ */
