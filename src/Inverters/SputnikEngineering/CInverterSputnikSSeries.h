/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2009-2012 Tobias Frost

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ----------------------------------------------------------------------------
 */

/** \file CInverterSputnikSSeries.h
 *
 *  Created on: May 21, 2009
 *      Author: Tobias Frost
 *
 *      Contributors:
 *      E.A.Neonakis <eaneonakis@freemail.gr>
 */

#ifndef CINVERTERSPUTNIKSSERIES_H_
#define CINVERTERSPUTNIKSSERIES_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined HAVE_INV_SPUTNIK

#include "Inverters/interfaces/InverterBase.h"
#include "Inverters/BasicCommands.h"

#include "Inverters/SputnikEngineering/SputnikCommand/ISputnikCommand.h"

#include <queue>
#include <set>

/** \fixme Implements the Inverter Interface for the Sputnik S Series
 *
 * The Sputnik S-Series are an inverter family by Sputnik Engineering
 * Please see the manufactor's homepage for details.
 */
class CInverterSputnikSSeries: public IInverterBase
{
public:
	CInverterSputnikSSeries(const string & name,
			const string & configurationpath);
	virtual ~CInverterSputnikSSeries();

	virtual bool CheckConfig();

	/** implements the ICommandTarget interface */
	virtual void ExecuteCommand(const ICommand *Command);

protected:
	/** calculate the checksum for the telegram stored in str */
	static unsigned int CalcChecksum(const char* str, int len);

private:

	/// Commands for the Workscheduler
	enum Commands
	{
		CMD_INIT = 1000,
		CMD_WAIT4CONNECTION,
		CMD_IDENTFY_WAIT,
		CMD_POLL,
		CMD_WAIT_RECEIVE,
		CMD_DISCONNECTED,
		CMD_DISCONNECTED_WAIT,
		CMD_EVALUATE_RECEIVE,
		CMD_WAIT_SENT,
		CMD_SEND_QUERIES,
		CMD_QUERY_POLL
	};

	/// Dataports of the sputnik inverters.
	enum Ports
	{
		QUERY = 100, COMMAND = 200, ALARM = 300,
		INTERFACE = 1000
	};

	/// known queries
	enum query
	{
		TYP,	SWV,	BUILDVER,	EC00,	EC01,	EC02,
		EC03,	EC04,	EC05,	EC06,	EC07,	EC08,	PAC,
		KHR,	DYR,	DMT,	DDY,	KYR,	KMT,	KDY,
		KT0,	PIN,	TNF,	PRL,	UDC,	UL1,	UL2,
		UL3,	IDC,	IL1,	IL2,	IL3,	TKK,	TK2,
		TK3,	TMI,	THR,	SYS,    IEE,    IEA,    IED,
                PDC,    UGD,    KLD,    CAC
	};

	/// Build up the communication string
	///
	/// \returns the string created, or "" if nothing to do.
	string assemblequerystring();

	/// parse the answer of the inverter.
	int parsereceivedstring(const string& s);

	/// helper for parsereceivedstring()
	bool parsetoken(string token);

	/// FIFO for commands to be transmitted.
	queue<enum query> cmdqueue;

	/// Adress to use as "our" adress for communication
	/// This can be set by the conffile and the parameter ownadr
	/// defaults to 0xFB
	/// unsigned int ownadr;
	void tokenizer(const char *delimiters, const string& s,
			vector<string> &tokens);

	/// cache for inverters comm adr.
	unsigned int commadr;
	/// cache for own adr
	unsigned int ownadr;

    /// stores supported commands.
    vector<ISputnikCommand*> commands;

    /// stores pending commmands.
    vector<ISputnikCommand*> pendingcommands;

    /// stores not answered commands (by removing the ansewered ones)
    set<ISputnikCommand*> notansweredcommands;
};

#endif

#endif /* CINVERTERSPUTNIKSSERIES_H_ */
