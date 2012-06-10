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

#ifndef CINVERTERSPUTNIKSSERIESSIMULATOR_H_
#define CINVERTERSPUTNIKSSERIESSIMULATOR_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined HAVE_INV_SPUTNIKSIMULATOR

#include "Inverters/interfaces/InverterBase.h"
#include "Inverters/BasicCommands.h"

#include "Inverters/SputnikEngineering/SputnikCommand/ISputnikCommand.h"

#include <set>

/** Implements a (simple) simulator for the Sputnik S Series
 *
 * The Sputnik S-Series are an inverter family by Sputnik Engineering
 * Please see the manufactor's homepage for details.
 */
class CInverterSputnikSSeriesSimulator: public IInverterBase
{
public:
    struct simulator_commands {
        const char *token;
        float scale1;
        IValue *value;
        float scale2;
        IValue *value2;
    };

public:

	CInverterSputnikSSeriesSimulator(const string & name,
			const string & configurationpath);
	virtual ~CInverterSputnikSSeriesSimulator();

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
		CMD_CONNECTED,
		CMD_EVALUATE_RECEIVE,
		CMD_WAIT_SENT,
	};

	/// Dataports of the sputnik inverters.
	enum Ports
	{
		QUERY = 100, COMMAND = 200, ALARM = 300,
		INTERFACE = 1000
	};

	/// parse the answer of the inverter.
	std::string parsereceivedstring(const string& s);

	/// helper for parsereceivedstring()
	bool parsetoken(string token);

	/// Adress to use as "our" adress for communication
	/// This can be set by the conffile and the parameter ownadr
	/// defaults to 0xFB
	/// unsigned int ownadr;
	void tokenizer(const char *delimiters, const string& s,
			vector<string> &tokens);

	/// cache for inverters comm adr.
	unsigned int commadr;

    struct simulator_commands *scommands;
};

#endif

#endif /*CINVERTERSPUTNIKSSERIESSIMULATOR_H_*/
