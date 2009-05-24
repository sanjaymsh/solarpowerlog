/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CInverterSputnikSSeries.h), the license terms are:

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

/** \file CInverterSputnikSSeries.h
 *
 *  Created on: May 21, 2009
 *      Author: tobi
 */

#ifndef CINVERTERSPUTNIKSSERIES_H_
#define CINVERTERSPUTNIKSSERIES_H_


/** \fixme COMMENT ME
 *
 *
 * TODO DOCUMENT ME!
 */
#include "InverterBase.h"
#include "Inverters/BasicCommands.h"

#include <queue>

class CInverterSputnikSSeries: public IInverterBase {
public:
	CInverterSputnikSSeries(const string & name, const string & configurationpath);
	virtual ~CInverterSputnikSSeries();

	virtual bool CheckConfig();

	virtual void ExecuteCommand(const ICommand *Command);

protected:
	/* calculate the checksum for the telegramm stored in str.
	 * note: */
	static unsigned int CalcChecksum(const char* str, int len);



private:

	/// Commands for the Workscheduler
	enum Commands
	{
		CMD_INIT = 1000,
		CMD_IDENTFY_WAIT,
		CMD_POLL,
		CMD_WAIT_RECEIVE,
		CMD_DISCONNECTED,

	};

	// Dataports of the sputnik inverters.

	enum Ports
	{
		QUERY = 100,
		COMMAND = 200,
		ALARM = 300, // told that the device reports errors on this ports.
		INTERFACE = 1000
	};


	enum query {
		TYP,
		SWVER,
		BUILDVER,
		EC00,
		EC01,
		EC02,
		EC03,
		EC04,
		EC05,
		EC06,
		EC07,
		EC08,
	};

	void pushinverterquery(enum query q);

	string assemblequerystring();

	queue<enum query> cmdqueue;

	/// Adress to use as "our" adress for communication
	/// This can be set by the conffile and the parameter ownadr
	/// defaults to 0xFB
	/// unsigned int ownadr;

};


#endif /* CINVERTERSPUTNIKSSERIES_H_ */
