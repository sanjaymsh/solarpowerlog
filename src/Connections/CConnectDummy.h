/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CConnectDummy.h), the license terms are:

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

/** \file CConnectDummy.h
 *
 *  Created on: May 22, 2009
 *      Author: tobi
 */

#ifndef CCONNECTDUMMY_H_
#define CCONNECTDUMMY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/** This is a dummy connection class which only fills the gap if
 * - the instanciator does not need comms  but the
 * base class wants a instance. (e.g datafilters)
 */
#include "interfaces/IConnect.h"

class CConnectDummy: public IConnect {

protected:
	friend class IConnectFactory;
	CConnectDummy(const string &configurationname);

public:
	virtual ~CConnectDummy();

	/// Connect to something
	/// NOTE: Needed to be overriden! ALWAYS Open in a NON_BLOCK way, or implement a worker thread
	virtual bool Connect(ICommand *) {return false;};
	/// Tear down the connection.
	virtual bool Disconnect(  ICommand * ) {return false;};

	/// Send a array of characters (can be used as binary transport, too)
	virtual bool Send(const char */*tosend*/ , unsigned int /*len*/, ICommand * /*callback*/ = NULL) { return false; };
	/// Send a strin Standard implementation only wraps to above Send.
	///
	/// Receive a string. Do now get more than maxxsize (-1 == no limit)
	/// NOTE:
	virtual bool Receive( ICommand *) { return false; };

	virtual bool CheckConfig(void) ;

	virtual void _main (void) {};

};

#endif /* CCONNECTDUMMY_H_ */
