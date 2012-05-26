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

// Experimental -- currently being coded.
// #define SPUTNIK_USE_NEW_COMMAND_HANDLING

#include "Inverters/interfaces/InverterBase.h"
#include "Inverters/BasicCommands.h"

#include "Inverters/SputnikEngineering/SputnikCommand/ISputnikCommand.h"

#include <queue>

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
	/** calculate the checksum for the telegramm stored in str */
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
		CMD_QUERY_IDENTIFY,
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

	/// Add a inverter-query into the queue for later quering...
	void pushinverterquery(enum query q);

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

	// token handlers
	// they all will take its tokens and do whatever required, usually
	// updating Capabilities.
	bool token_TYP(const vector<string> &tokens);
	bool token_SWVER(const vector<string> &tokens);
	bool token_BUILDVER(const vector<string> &tokens);
	bool token_ECxx(const vector<string> &tokens);
	bool token_PAC(const vector<string> &tokens);
	bool token_PDC(const vector<string> &tokens);
	bool token_KHR(const vector<string> &tokens);
	bool token_DYR(const vector<string> &tokens);
	bool token_DMT(const vector<string> &tokens);
	bool token_DDY(const vector<string> &tokens);
	bool token_KYR(const vector<string> &tokens);
	bool token_KMT(const vector<string> &tokens);
	bool token_KDY(const vector<string> &tokens);
	bool token_KT0(const vector<string> &tokens);
	bool token_PIN(const vector<string> &tokens);
	bool token_TNF(const vector<string> &tokens);
	bool token_PRL(const vector<string> &tokens);
	bool token_UDC(const vector<string> &tokens);
	bool token_UL1(const vector<string> &tokens);
	bool token_UL2(const vector<string> &tokens);
	bool token_UL3(const vector<string> &tokens);
	bool token_IDC(const vector<string> &tokens);
	bool token_IL1(const vector<string> &tokens);
	bool token_IL2(const vector<string> &tokens);
	bool token_IL3(const vector<string> &tokens);
	bool token_TKK(const vector<string> &tokens);
	bool token_TK2(const vector<string> &tokens);
	bool token_TK3(const vector<string> &tokens);
	bool token_TMI(const vector<string> &tokens);
	bool token_THR(const vector<string> &tokens);
	bool token_SYS(const vector<string> &tokens);
	bool token_IEE(const vector<string> &tokens);
	bool token_IED(const vector<string> &tokens);
	bool token_IEA(const vector<string> &tokens);
    bool token_UGD(const vector<string> &tokens);
    bool token_KLD(const vector<string> &tokens);
    bool token_CAC(const vector<string> &tokens);

	// a helper, as this is shared by two token parsers.
	void create_versioncapa(void);

	///  some internal infos we cache....
	int swversion;
	int swbuild;

	/// cache for inverters comm adr.
	unsigned int commadr;
	/// cache for own adr
	unsigned int ownadr;

	/// helper to detect status code changes.
	unsigned int laststatuscode;

	/// helper for Execute Command to store error detection / recovery (errorcounter)
	unsigned int errcnt_;

#ifdef SPUTNIK_USE_NEW_COMMAND_HANDLING
    /// stores supported commands.
    vector<ISputnikCommand*> commands;

    /// stores pending commmands.
    vector<ISputnikCommand*> pendingcommands;
#endif
};

#endif

#endif /* CINVERTERSPUTNIKSSERIES_H_ */
