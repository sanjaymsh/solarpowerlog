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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>

#include "configuration/ILogger.h"

using namespace std;


/** Inteface for all communication classes
 *
 * This Interface is the API for all concrete comm methods.
 * The class is abstract, so it cannot be instanciated by itself.
 *
 * Anyway, it is intended, that the class is only created by the IConnectFactory.
 *
*/
class IConnect {
public:
	/// Constructor.
	///
	/// The constructor gets the configuration path to be used to extract
	/// its configuration.
	///
	IConnect(const string &configurationname);

	/// Setp the Logger.
	virtual void SetupLogger(const string& parentlogger, const string &spec = "Comms");

	/// Desctructor
	virtual ~IConnect();

	/// Connect to something
	/// NOTE: Needed to be overriden! ALWAYS Open in a NON_BLOCK way, or implement a worker thread
	virtual bool Connect() = 0;
	/// Tear down the connection.
	virtual bool Disconnect() = 0;

	/// Send a array of characters (can be used as binary transport, too)
	///
	/// \warning not all classes might understand binary transport. Check
	/// the concrete implmentation for details!
	/// \param tosend what to send
	/// \param len how many bytes
	/// \returns true on success, false on error.
	virtual bool Send(const char *tosend, unsigned int len) = 0;

	/// Send a string
	/// \note Standard implementation only wraps to above Send-binray.
	///
	/// \param tosend std::string to send
	/// \returns true on success, false on error.
	/// Override for better performance!
	virtual bool Send(const string& tosend) {
		return Send(tosend.c_str(),tosend.length());
	}

	/// Receive a string. Do now get more than maxxsize (-1 == no limit)
	///
	virtual bool Receive(string &wheretoplace) = 0;

	/// Check the configuration for validty. Retrurn false on config errors.
	/// (programm will abort then!)
	virtual bool CheckConfig(void) = 0 ;

	/// Check if we believe the connection is still active
	/// Note: if the concrete implementation cannot tell,
	/// it should always return true, as the default implementaion does.
	/// (the inverter class has to do some kind of timeout-handling anyway)
	virtual bool IsConnected(void) { return true; }

protected:
	/// Storage for the Configuration Path to extract settings.
	string ConfigurationPath;

	/// Associated logger.
	/// \note: By default, it will attached to the root logger. If unwanted,
	/// use the SetupLogger call. (As IInverterBase derived classes do)
	ILogger logger;

};

#endif /* ICONNECT_H_ */
