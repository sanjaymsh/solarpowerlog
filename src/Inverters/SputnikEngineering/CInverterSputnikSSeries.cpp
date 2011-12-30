/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CInverterSputnikSSeries.cpp), the license terms are:

 You can redistribute it and/or modify it under the terms of the GNU
 General Public License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

// HOW TO HANLDE MULTI-PHASE UNITS
// not implemented, but some ideas:
// -- for all parameters, which are dependend on the phase,
//    create extra parameters which the appendix _L[1-3]
// -- create a filter, a "multi-phase-data-splitter", and connect it
//    to the inverter.
// -- it will search all capabilities for the phase variants and, if found,
//	  it will locally "rename" the capability for the following filters and displays.
// --> The phases will be transformed to new data source, like a virtual inverter.

/** \file CInverterSputnikSSeries.cpp
 *
 *  Created on: May 21, 2009
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_INV_SPUTNIK

#include "configuration/Registry.h"
#include "configuration/CConfigHelper.h"

#include "CInverterSputnikSSeries.h"

#include <libconfig.h++>
#include <iostream>
#include <sstream>
#include <cstring>

#include "patterns/ICommand.h"

#include "interfaces/CWorkScheduler.h"

#include "Inverters/Capabilites.h"
#include "patterns/CValue.h"

#include "configuration/ILogger.h"

#include <cstring>

using namespace libconfig;

static struct
{
	unsigned int typ;
	const char *description;
}
		model_lookup[] = {
				{ 2001, "SolarMax 2000 E" },
				{ 3001, "SolarMax 3000 E" },
				{ 4000, "SolarMax 4000 E" },
				{ 6000, "SolarMax 6000 E" },
				{ 2010, "SolarMax 2000 C" },
				{ 3010, "SolarMax 3000 C" },
				{ 4010, "SolarMax 4000 C" },
				{ 4200, "SolarMax 4200 C" },
				{ 6010, "SolarMax 6000 C" },
				{ 20010, "SolarMax 2000 S" },
				{ 20020, "SolarMax 3000 S" },
				{ 20030, "SolarMax 4200 S" },
				{ 20040, "SolarMax 6000 S" },
				{ 20, "SolarMax 20 C" },
				{ 25, "SolarMax 25 C" },
				{ 30, "SolarMax 30 C" },
				{ 35, "SolarMax 35 C" },
				{ 50, "SolarMax 50 C" },
				{ 80, "SolarMax 80 C" },
				{ 100, "SolarMax 100 C" },
				{ 300, "SolarMax 300 C" },
				{
						-1,
						"UKNOWN MODEL. PLEASE FILE A BUG WITH THE REPORTED ID, ALONG WITH ALL INFOS YOU HAVE" } };

static struct
{
	unsigned int code;
	enum InverterStatusCodes status;
	const char *description;
}
		statuscodes[] = {
				{ 20002, NOT_FEEDING_OK, "Solar radiation too low" },
				{ 20003, NOT_FEEDING_OK, "Inverter Starting up" },
				{ 20004, FEEDING_MPP, "Feeding on MPP" },

				{ 20006, FEEDING_MAXPOWER, "Feeding. Inverter at power limit" },
				{ 20008, FEEDING, "Feeding" },

				{ 20115, NOT_FEEDING_EXTEVENT, "Off-grid" },
				{ 20116, NOT_FEEDING_EXTEVENT, "Grid Frequency too high" },
				{ 20117, NOT_FEEDING_EXTEVENT, "Grid Frequency too low" },

				{
						-1,
						STATUS_UNAVAILABLE,
						"Unknown Statuscode -- PLEASE FILE A BUG WITH AS MUCH INFOS AS YOU CAN FIND OUT -- BEST, READ THE DISPLAY OF THE INVERTER." }, };

using namespace std;

CInverterSputnikSSeries::CInverterSputnikSSeries(const string &name,
		const string & configurationpath) :
	IInverterBase::IInverterBase(name, configurationpath, "inverter")
{

	swversion = swbuild = 0;
	ownadr = 0xfb;
	commadr = 0x01;
	laststatuscode = (unsigned int) -1;
	errcnt_ = 0;

	// Add the capabilites that this inverter has
	// Note: The "must-have" ones CAPA_CAPAS_REMOVEALL and CAPA_CAPAS_UPDATED are already instanciated by the base class constructor.
	// Note2: You also can add capabilites as soon you know them (runtime detection)

	string s;
	IValue *v;
	CCapability *c;
	s = CAPA_INVERTER_MANUFACTOR_NAME;
	v = IValue::Factory(CAPA_INVERTER_MANUFACTOR_TYPE);
	((CValue<string>*) v)->Set("Sputnik Engineering");
	c = new CCapability(s, v, this);
	AddCapability(c);

	// Add the request to initialize as soon as the system is up.
	ICommand *cmd = new ICommand(CMD_INIT, this);
	Registry::GetMainScheduler()->ScheduleWork(cmd);

	CConfigHelper cfghlp(configurationpath);
	float interval;
	cfghlp.GetConfig("queryinterval", interval, 5.0f);

	// Query settings needed and default all optional settings.
	cfghlp.GetConfig("ownadr", ownadr, 0xFBu);
	cfghlp.GetConfig("commadr", commadr, 0x01u);

	s = CAPA_INVERTER_QUERYINTERVAL;
	v = IValue::Factory(CAPA_INVERTER_QUERYINTERVAL_TYPE);
	((CValue<float>*) v)->Set(interval);
	c = new CCapability(s, v, this);
	AddCapability(c);

	s = CAPA_INVERTER_CONFIGNAME;
	v = IValue::Factory(CAPA_INVERTER_CONFIGNAME_TYPE);
	((CValue<std::string>*) v)->Set(name);
	c = new CCapability(s, v, this);
	AddCapability(c);

	LOGDEBUG(logger,"Inverter configuration:");
	LOGDEBUG(logger,"class CInverterSputnikSSeries ");
	LOGDEBUG(logger,"Query Interval: "<< interval);
	LOGDEBUG(logger,"Ownadr: " << ownadr << " Commadr: " << commadr);
	cfghlp.GetConfig("comms", s, (string) "unset");
	LOGDEBUG(logger,"Communication: " << s);

}

CInverterSputnikSSeries::~CInverterSputnikSSeries()
{
	// TODO Auto-generated destructor stub
}

bool CInverterSputnikSSeries::CheckConfig()
{
	string setting;
	string str;

	bool fail = false;

	CConfigHelper hlp(configurationpath);
	fail |= (true != hlp.CheckConfig("comms", Setting::TypeString));
	// Note: Queryinterval is optional. But CConfigHelper handle also opt.
	// parameters and checks for type.
	fail
			|= (true != hlp.CheckConfig("queryinterval", Setting::TypeFloat,
					true));
	fail |= (true != hlp.CheckConfig("commadr", Setting::TypeInt));

	// Check config of the component, if already instanciated.
	if (connection) {
		fail |= (true != connection->CheckConfig());
	}

	LOGTRACE(logger, "Check Configuration result: " << !fail);
	return !fail;
}

/** Calculate the telegram checksum and return it.
 *
 * The sputnik protects it telegrams with a checksum.
 * The checksum is a sum of all bytes of a telegram,
 * starting after the { and ending at the | just before
 * the checksum.
 *
 * This routine assumes, that you give it the complete
 * telegram, and only the checkszum and the } is missing.
 */
unsigned int CInverterSputnikSSeries::CalcChecksum(const char *str, int len)
{
	unsigned int chksum = 0;
	str++;
	do {
		chksum += *str++;
	} while (--len);

	return chksum;
}

void CInverterSputnikSSeries::ExecuteCommand(const ICommand *Command)
{
	bool dbg = true;

	string commstring = "";
	string reccomm = "";
	ICommand *cmd;
	timespec ts;

	switch ((Commands) Command->getCmd())
	{

	case CMD_DISCONNECTED:
	{
		// DISCONNECTED: Error detected, the link to the com partner is down.
		// Action: Schedule connection retry in xxx seconds
		// Next-State: INIT (Try to connect)
		LOGDEBUG(logger, "new state: CMD_DISCONNECTED");

        // Timeout on reception
		// we assume that we are now disconnected.
		// so lets schedule a reconnection.
		// TODO this time should be configurable.
		cmd = new ICommand(CMD_DISCONNECTED_WAIT, this);
		connection->Disconnect(cmd);

		// Tell everyone that all data is now invalid.
		CCapability *c = GetConcreteCapability(CAPA_INVERTER_DATASTATE);
		CValue<bool> *v = (CValue<bool> *) c->getValue();
		v->Set(false);
		c->Notify();
		break;
	}

	case CMD_DISCONNECTED_WAIT:
	{
		LOGTRACE(logger, "new state: CMD_DISCONNECTED_WAIT");
		cmd = new ICommand(CMD_INIT, this);
		timespec ts;
		ts.tv_sec = 15;
		ts.tv_nsec = 0;
		Registry::GetMainScheduler()->ScheduleWork(cmd, ts);
		break;
	}

	case CMD_INIT:
	{
		// INIT: Try to connect to the comm partner
		// Action Connection Attempt
		// Next-State: Wait4Connection
		LOGDEBUG(logger, "new state: CMD_INIT");

		cmd = new ICommand(CMD_WAIT4CONNECTION, this);
		connection->Connect(cmd);
		break;

		// storage of objects in boost::any
		//cmd->addData("TEST", cmd);
		//cmd = boost::any_cast<ICommand*>(cmd->findData("TEST"));
	}

	case CMD_WAIT4CONNECTION:
	{
		LOGDEBUG(logger, "new state: CMD_WAIT4CONNECTION");

		int err = -1;
		// WAIT4CONNECTION: Wait until connection is up of failed to set up
		// by the communication object.
		// Action: Check success/error flag
		// Next-State: Depending on success:
		//		success IDENTIFY_COMM
		//		error 	DISCONNECTED
		try {
			err = boost::any_cast<int>(Command->findData(ICMD_ERRNO));
		} catch (...) {
			LOGDEBUG(logger,"CMD_WAIT4CONNECTION: unexpected exception");
			err = -1;
		}

		if (err < 0) {
			try {
				LOGERROR(logger, "Error while connecting: " <<
						boost::any_cast<string>(Command->findData(ICMD_ERRNO_STR)));
			} catch (...) {
				LOGERROR(logger, "Unknown error while connecting.");
			}

			cmd = new ICommand(CMD_DISCONNECTED, this);
			Registry::GetMainScheduler()->ScheduleWork(cmd);
		} else {
			cmd = new ICommand(CMD_QUERY_IDENTIFY, this);
			Registry::GetMainScheduler()->ScheduleWork(cmd);
		}
	}
		break;

	case CMD_QUERY_IDENTIFY:
		LOGDEBUG(logger, "new state: CMD_QUERY_IDENTIFY ");

		pushinverterquery(TYP);
		pushinverterquery(SWV);
		pushinverterquery(BUILDVER);
		/// this fall through is intended.

	case CMD_QUERY_POLL:
		LOGDEBUG(logger, "new state: CMD_QUERY_POLL ");

		pushinverterquery(PAC);
		pushinverterquery(KHR);
		pushinverterquery(PIN);
		pushinverterquery(KT0);
		pushinverterquery(DDY);
		pushinverterquery(KYR);
		pushinverterquery(KMT);
		pushinverterquery(KDY);
		pushinverterquery(KT0);
		pushinverterquery(PRL);

		pushinverterquery(UDC);
		pushinverterquery(IDC);
		pushinverterquery(IL1);
		pushinverterquery(UL1);
		pushinverterquery(TKK);
		pushinverterquery(TMI);
		pushinverterquery(THR);
		pushinverterquery(TNF);
		pushinverterquery(SYS);
		/// this fall through is intended.

	case CMD_SEND_QUERIES:
	{
		LOGDEBUG(logger, "new state: CMD_SEND_QUERIES ");

		commstring = assemblequerystring();
		LOGTRACE(logger, "Sending: " << commstring << " Len: "<< commstring.size());

		cmd = new ICommand(CMD_WAIT_SENT, this);
		cmd->addData(ICONN_TOKEN_SEND_STRING, commstring);
		connection->Send(cmd);
	}
		break;

	case CMD_WAIT_SENT:
	{
		LOGDEBUG(logger, "new state: CMD_WAIT_SENT");
		int err;
		try {
			err = boost::any_cast<int>(Command->findData(ICMD_ERRNO));
		} catch (...) {
			LOGDEBUG(logger, "BUG: Unexpected exception.");
			err = -1;
		}

		if (err < 0) {
			cmd = new ICommand(CMD_DISCONNECTED, this);
			Registry::GetMainScheduler()->ScheduleWork(cmd);
			break;
		}
	}
	// Fall through ok.

	case CMD_WAIT_RECEIVE:
	{
		LOGDEBUG(logger, "new state: CMD_WAIT_RECEIVE");

		cmd = new ICommand(CMD_EVALUATE_RECEIVE, this);
		connection->Receive(cmd);
	}
		break;

	case CMD_EVALUATE_RECEIVE:
	{
		LOGDEBUG(logger, "new state: CMD_EVALUATE_RECEIVE");

		int err;
		std::string s;
		try {
			err = boost::any_cast<int>(Command->findData(ICMD_ERRNO));
		} catch (...) {
			LOGDEBUG(logger, "BUG: Unexpected exception.");
			err = -1;
		}

		if (err < 0) {
			// we do not differenziate the error here, a error is a error....
			cmd = new ICommand(CMD_DISCONNECTED, this);
			Registry::GetMainScheduler()->ScheduleWork(cmd);
			try {
				s = boost::any_cast<std::string>(Command->findData(
						ICMD_ERRNO_STR));
				LOGERROR(logger, "Receive Error: " << s);
			} catch (...) {
				LOGERROR(logger, "Receive Error: " << strerror(-err));
			}
			break;
		}

		try {
			s = boost::any_cast<std::string>(Command->findData(
					ICONN_TOKEN_RECEIVE_STRING));
		} catch (...) {
			LOGDEBUG(logger, "Unexpected Exception");
			break;
		}

		LOGTRACE(logger, "Received :" << s << "len: " << s.size());
		if (logger.IsEnabled(ILogger::LL_TRACE)) {
			string st;
			char buf[32];
			for (unsigned int i = 0; i < s.size(); i++) {
				sprintf(buf, "%02x", (unsigned char) s[i]);
				st = st + buf;
				if (i && i % 16 == 0)
					st = st + "\n";
				else
					st = st + ' ';
			}
			LOGTRACE(logger, "Received in hex: "<< st );
		}

		int parseresult = parsereceivedstring(s);
		if (0 > parseresult ) {
			// Reconnect on parse errors.
			LOGERROR(logger, "Parse error while receiving.");
			LOGDEBUG(logger, "Received: " << s);
			cmd = new ICommand(CMD_DISCONNECTED, this);
			Registry::GetMainScheduler()->ScheduleWork(cmd);
			break;
		} else if ( parseresult == 0) {
			// The received data seems not to be for our inverter.
			// Can happen on a shared connection.
			// So lets wait again.

			ICommand *cmd = new ICommand(CMD_EVALUATE_RECEIVE, this);
			connection->Receive(cmd);
			break;
		}

		// check if there are queries left in this cycle.
		if (!cmdqueue.empty()) {
			cmd = new ICommand(CMD_SEND_QUERIES, this);
			// there are more. finish them before setting the data valid.
			Registry::GetMainScheduler()->ScheduleWork(cmd);
			break;
		}

		// TODO differenciate between identify query and "normal" runtime queries
		cmd = new ICommand(CMD_QUERY_POLL, this);

		CCapability *c = GetConcreteCapability(CAPA_INVERTER_DATASTATE);
		CValue<bool> *vb = (CValue<bool> *) c->getValue();
		vb->Set(true);
		c->Notify();

		c = GetConcreteCapability(CAPA_INVERTER_QUERYINTERVAL);
		CValue<float> *v = (CValue<float> *) c->getValue();
		ts.tv_sec = v->Get();
		ts.tv_nsec = ((v->Get() - ts.tv_sec) * 1e9);
		Registry::GetMainScheduler()->ScheduleWork(cmd, ts);
	}
		break;

	default:
		LOGFATAL(logger, "Unknown CMD received");
		abort();
		break; // to have one code-analysis warning less.
	}

}

void CInverterSputnikSSeries::pushinverterquery(enum query q)
{
	cmdqueue.push(q);
}

string CInverterSputnikSSeries::assemblequerystring()
{
	if (cmdqueue.empty()) {
		LOGTRACE(logger, "assemblequerystring: empty task lisk.");
		return "";
	}

	int len = 0;
	int expectedanswerlen = 0;
	int currentport = 0;
	string nextcmd;
	string querystring, tmp;
	bool cont = true;
	char formatbuffer[256];

	do {
		switch (cmdqueue.front())
		{

		case TYP:
		{
			if (currentport && currentport != QUERY) {
				// Changing ports are not supported.
				// Different ports means a different message.
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "TYP";
			expectedanswerlen += 9;
			break;
		}

		case SWV:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "SWV";
			expectedanswerlen += 7;
			break;
		}

		case BUILDVER:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "BDN";
			expectedanswerlen += 9;
			break;
		}

		case EC00:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "EC00";
			expectedanswerlen += 27;
			break;
		}

		case EC01:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "EC01";
			expectedanswerlen += 27;
			break;
		}

		case EC02:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "EC02";
			expectedanswerlen += 27;
			break;
		}

		case EC03:
		{

			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "EC03";
			expectedanswerlen += 27;
			break;
		}

		case EC04:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "EC04";
			expectedanswerlen += 27;
			break;
		}

		case EC05:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "EC05";
			expectedanswerlen += 27;
			break;
		}

		case EC06:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "EC06";
			expectedanswerlen += 27;
			break;
		}

		case EC07:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "EC07";
			expectedanswerlen += 27;
			break;
		}

		case EC08:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "EC08";
			expectedanswerlen += 27;
			break;
		}

		case PAC:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "PAC";
			expectedanswerlen += 9;
			break;
		}

		case KHR:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "KHR";
			expectedanswerlen += 9;
			break;
		}

		case DYR:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "DYR";
			expectedanswerlen += 7;
			break;
		}

		case DMT:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "DMT";
			// TODO check answer len
			expectedanswerlen += 7;
			break;
		}

		case DDY:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "DDY";
			expectedanswerlen += 7;
			break;
		}

		case KYR:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "KYR";
			expectedanswerlen += 9;
			break;
		}

		case KMT:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "KMT";
			expectedanswerlen += 7;
			break;
		}

		case KDY:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "KDY";
			expectedanswerlen += 10;
			break;
		}

		case KT0:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "KT0";
			expectedanswerlen += 10;
			break;
		}

		case PIN:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "PIN";
			expectedanswerlen += 9;
			break;
		}

		case TNF:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "TNF";
			// TODO check answer len
			expectedanswerlen += 10;
			break;
		}

		case PRL:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "RPL";
			// TODO check answer len
			expectedanswerlen += 10;
			break;
		}

		case UDC:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "UDC";
			// TODO check answer len
			expectedanswerlen += 10;
			break;
		}

		case UL1:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "UL1";
			// TODO check answer len
			expectedanswerlen += 10;
			break;
		}

		case UL2:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "UL2";
			// TODO check answer len
			expectedanswerlen += 10;
			break;
		}

		case UL3:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "UL3";
			// TODO check answer len
			expectedanswerlen += 10;
			break;
		}

		case IDC:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "IDC";
			// TODO check answer len
			expectedanswerlen += 10;
			break;
		}

		case IL1:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "IL1";
			// TODO check answer len
			expectedanswerlen += 10;
			break;
		}

		case IL2:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "IL2";
			// TODO check answer len
			expectedanswerlen += 10;
			break;
		}

		case IL3:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "IL3";
			// TODO check answer len
			expectedanswerlen += 10;
			break;
		}

		case TKK:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "TKK";
			// TODO check answer len
			expectedanswerlen += 10;
			break;
		}

		case TK2:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "TK2";
			// TODO check answer len
			expectedanswerlen += 10;
			break;
		}

		case TK3:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "TK3";
			// TODO check answer len
			expectedanswerlen += 10;
			break;
		}

		case TMI:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "TMI";
			// TODO check answer len
			expectedanswerlen += 10;
			break;
		}

		case THR:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "THR";
			// TODO check answer len
			expectedanswerlen += 10;
			break;
		}

		case SYS:
		{
			if (currentport && currentport != QUERY) {
				cont = false;
				break;
			}
			currentport = QUERY;
			nextcmd = "SYS";
			// TODO check answer len
			expectedanswerlen += 10;
			break;
		}

		}

		if (cont) {
			// check if command will fit into max. telegram len
			// note: 255 = max len, 15 = header "{xx;yy;zz|pppp:", 6 = tail "|CHKS}"
			// (plus one byte for the seperator.)
			// note: this is not squeezed to its end, as we will usually not
			// fill up to 255 bytes because lacking commands...
			if (nextcmd.length() + len <= (255 - 15 - 6) - 1 &&
			// use our assumption to limit the expected receive telegram
					// add a safety margin of 10.
					expectedanswerlen <= (255 - 15 - 6 - 1 - 10)) {
				// Check if we need to insert a seperator.
				if (len) {
					querystring += ";";
					len++;
				}
				// Add the prepared command and remove it from the queue.
				querystring += nextcmd;
				len += nextcmd.length();
				cmdqueue.pop();
			} else {
				cont = false;
			}
		}
	} while (cont && !cmdqueue.empty());

	sprintf(formatbuffer, "%X:", currentport);
	len = strlen(formatbuffer) + querystring.length() + 10 + 6;

	// finally prepare Header "{<from>;<to>;<len>|<port>:"
	sprintf(formatbuffer, "{%02X;%02X;%02X|%X:", ownadr, commadr, len,
			currentport);

	tmp = formatbuffer + querystring + '|';
	querystring = tmp;

	sprintf(formatbuffer, "%04X}", CalcChecksum(querystring.c_str(),
			querystring.length()));

	querystring += formatbuffer;
	return querystring;
}

int CInverterSputnikSSeries::parsereceivedstring(const string & s)
{

	unsigned int i;

	// check for basic constraints...
	if (s[0] != '{' || s[s.length() - 1] != '}')
		return -1;

	// tokenizer (taken from
	// http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html
	// but  modified.

	// we take the received string and "split" it into smaller pieces.
	// the pieces ("tokens") then assemble one single information to be
	// for this, we split by:
	// ";" "|" ":" (and "{}")

	vector<string> tokens;
	char delimiters[] = "{;|:}";
	tokenizer(delimiters, s, tokens);

	// Debug: Print all received tokens
#if defined DEBUG_TOKENIZER
	if (logger.IsEnabled(ILogger::LL_TRACE)) {
		std::stringstream ss;
		vector<string>::iterator it;
		for (i = 0, it = tokens.begin(); it != tokens.end() - 1; it++) {
			ss << i++ << ": " << (*it) << "\tlen: "
			<< (*it).length() << endl;
		}
		LOGTRACE(logger,ss);
	}
#endif

	unsigned int tmp;
	if (1 != sscanf(tokens.back().c_str(), "%x", &tmp)) {
		LOGDEBUG(logger, "could not parse checksum. Token was:" );
		return -1;
	}

	if (tmp != CalcChecksum(s.c_str(), s.length() - 6)) {
		LOGDEBUG(logger, "Checksum error on received telegram");
		return -1;
	}

	if (1 != sscanf(tokens[0].c_str(), "%x", &tmp)) {
		LOGDEBUG(logger, " could not parse from address");
		return -1;
	}

	if (tmp != commadr) {
		LOGDEBUG(logger, "Received string is not for us: Wrong Sender");
		return 0;
	}

	if (1 != sscanf(tokens[1].c_str(), "%x", &tmp)) {
		LOGDEBUG(logger, "could not parse to-address");
		return -1;
	}

	if (tmp != ownadr) {
		LOGDEBUG(logger, "Received string is not for us: Wrong receiver");
		return 0;
	}

	if (1 != sscanf(tokens[2].c_str(), "%x", &tmp)) {
		LOGDEBUG(logger, "could not parse telegram length");
		return -1;
	}

	if (tmp != s.length()) {
		LOGDEBUG(logger, "wrong telegram length ");
		return -1;
	}

	// TODO FIXME: Currently the data port is simply "ignored"
	// parsetoken should get a second argument to get the port infos.

	int  ret = 1;
	for (i = 4; i < tokens.size() - 1; i++) {
		if (!parsetoken(tokens[i])) {
			LOGDEBUG(logger,
					"BUG: Parse Error at token " << tokens[i]
					<< ". Received: " << s << "If the token is unknown or you subject a bug, please report it giving the  token ans received string"
			);
			ret = -1;
		}
	}
	return ret;
}

bool CInverterSputnikSSeries::parsetoken(string token)
{

	vector<string> subtokens;
	const char delimiters[] = "=,";
	tokenizer(delimiters, token, subtokens);

	// TODO rewrite this section: Lookup the strings and functions in a table, call
	// by function pointer.

	if (subtokens[0] == "TYP") {
		return token_TYP(subtokens);
	}

	if (subtokens[0] == "SWV") {
		return token_SWVER(subtokens);
	}

	if (subtokens[0] == "BDN") {
		return token_BUILDVER(subtokens);
	}

	if (subtokens[0].substr(0, 2) == "EC") {
		return token_ECxx(subtokens);
	}

	if (subtokens[0] == "PAC") {
		return token_PAC(subtokens);
	}

	if (subtokens[0] == "KHR") {
		return token_KHR(subtokens);
	}

	if (subtokens[0] == "DYR") {
		return token_DYR(subtokens);
	}

	if (subtokens[0] == "DMT") {
		return token_DMT(subtokens);
	}

	if (subtokens[0] == "DDY") {
		return token_DDY(subtokens);
	}

	if (subtokens[0] == "KYR") {
		return token_KYR(subtokens);
	}

	if (subtokens[0] == "KMT") {
		return token_KMT(subtokens);
	}

	if (subtokens[0] == "KDY") {
		return token_KDY(subtokens);
	}

	if (subtokens[0] == "KT0") {
		return token_KT0(subtokens);
	}

	if (subtokens[0] == "PIN") {
		return token_PIN(subtokens);
	}

	if (subtokens[0] == "TNF") {
		return token_TNF(subtokens);
	}
	if (subtokens[0] == "PRL") {
		return token_PRL(subtokens);
	}
	if (subtokens[0] == "UDC") {
		return token_UDC(subtokens);
	}

	if (subtokens[0] == "UL1") {
		return token_UL1(subtokens);
	}

	if (subtokens[0] == "UL2") {
		return token_UL2(subtokens);
	}

	if (subtokens[0] == "UL3") {
		return token_UL3(subtokens);
	}

	if (subtokens[0] == "IDC") {
		return token_IDC(subtokens);
	}
	if (subtokens[0] == "IL1") {
		return token_IL1(subtokens);
	}

	if (subtokens[0] == "IL2") {
		return token_IL2(subtokens);
	}

	if (subtokens[0] == "IL3") {
		return token_IL3(subtokens);
	}

	if (subtokens[0] == "TKK") {
		return token_TKK(subtokens);
	}

	if (subtokens[0] == "TK2") {
		return token_TK2(subtokens);
	}

	if (subtokens[0] == "TK3") {
		return token_TK3(subtokens);
	}

	if (subtokens[0] == "TMI") {
		return token_TMI(subtokens);
	}

	if (subtokens[0] == "THR") {
		return token_THR(subtokens);
	}

	if (subtokens[0] == "SYS") {
		return token_SYS(subtokens);
	}

	return true;
}

void CInverterSputnikSSeries::tokenizer(const char *delimiters,
		const string& s, vector<string> &tokens)
{
	unsigned int i;

	string::size_type lastPos = 0;
	string::size_type pos = 0;

	i = 0;
	// Skip tokens at the start of the string
	do {
		if (s[lastPos] == delimiters[i]) {
			lastPos++;
			i = 0;
		}
	} while (++i < strlen(delimiters));

	pos = lastPos;

	// get the first substring by finding the "second" delimiter
	i = lastPos;

	do {
		unsigned int tmp;
		tmp = s.find_first_of(delimiters[i], lastPos);
		if (tmp < pos)
			pos = tmp;
	} while (++i < strlen(delimiters));

	while (s.length() > pos && s.length() > lastPos) {
		unsigned int tmp, tmp2;

		if (pos - lastPos) {
			tokens.push_back(s.substr(lastPos, pos - lastPos));
		}
		lastPos = pos;

		// Skip delimiters.
		i = 0;
		do {
			if (s[lastPos] == delimiters[i]) {
				lastPos++;
				i = 0;
			}

		} while (++i < strlen(delimiters));

		// Find next "delimiter"
		i = 0;
		tmp2 = -1;
		do {
			tmp = s.find_first_of(delimiters[i], lastPos);
			if (tmp < tmp2)
				tmp2 = tmp;
		} while (++i < strlen(delimiters));
		pos = tmp2;
	}

	// Check if we have an "end-token" (not seperated)
	if (lastPos != s.length()) {
		tokens.push_back(s.substr(lastPos, s.length() - lastPos));
	}
}

bool CInverterSputnikSSeries::token_TYP(const vector<string> & tokens)
{

	string strmodel;
	unsigned int i = 0;

	// Check syntax
	if (tokens.size() != 2)
		return false;
	unsigned int model;
	sscanf(tokens[1].c_str(), "%x", &model);

	do {
		if (model_lookup[i].typ == model)
			break;
	} while (model_lookup[++i].typ != (unsigned int) -1);

	if (model_lookup[i].typ == (unsigned int) -1) {
		LOGWARN(logger, "Identified a " << model_lookup[i].description);
		LOGWARN(logger, "Received TYP was " << tokens[0] << "=" << tokens[1]);
	}

	// lookup if we already know that information.
	CCapability *cap = GetConcreteCapability(CAPA_INVERTER_MODEL);

	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_MODEL;
		v = IValue::Factory(CAPA_INVERTER_MODEL_TYPE);
		((CValue<string>*) v)->Set(model_lookup[i].description);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if (cap->getValue()->GetType() == CAPA_INVERTER_MODEL_TYPE) {
		CValue<string> *val = (CValue<string>*) cap->getValue();
		if (model_lookup[i].description != val->Get()) {

			LOGDEBUG(logger, "WEIRD: Updating inverter type from "
					<< val->Get() << " to "
					<< model_lookup[i].description);

			val->Set(model_lookup[i].description);
			cap->Notify();
		}
	} else {
		LOGDEBUG(logger, "BUG: " << CAPA_INVERTER_MODEL << " not a string ");
	}

	return true;
}

bool CInverterSputnikSSeries::token_SWVER(const vector<string> & tokens)
{
	int tmp;
	string ver;

	// Check syntax
	if (tokens.size() != 2)
		return false;

	sscanf(tokens[1].c_str(), "%x", &tmp);

	// Been there, seen that.
	if (swversion == tmp)
		return true;
	swversion = tmp;

	create_versioncapa();

	return true;
}

bool CInverterSputnikSSeries::token_BUILDVER(const vector<string> & tokens)
{
	int tmp;
	string ver;

	// Check syntax
	if (tokens.size() != 2)
		return false;

	sscanf(tokens[1].c_str(), "%x", &tmp);

	// Been there, seen that.
	if (swbuild == tmp)
		return true;

	swbuild = tmp;
	create_versioncapa();

	return true;
}

bool CInverterSputnikSSeries::token_ECxx(const vector<string> & tokens)
{
	// FIXME TODO Implement me!
	// Will be implemented later, as currently not-so-important
	// (and just unsure, how to handle the different entries. Probably as an ring-
	// buffer, with just updating the last.
	return true;
}

bool CInverterSputnikSSeries::token_PAC(const vector<string> & tokens)
{
	if (tokens.size() != 2)
		return false;

	unsigned int pac;
	float fpac;
	sscanf(tokens[1].c_str(), "%x", &pac);

	fpac = pac / 2.0;

	// lookup if we already know that information.
	CCapability *cap = GetConcreteCapability(CAPA_INVERTER_ACPOWER_TOTAL);

	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_ACPOWER_TOTAL;
		v = IValue::Factory(CAPA_INVERTER_ACPOWER_TOTAL_TYPE);
		((CValue<float>*) v)->Set(fpac);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if (cap->getValue()->GetType() == CAPA_INVERTER_ACPOWER_TOTAL_TYPE) {
		CValue<float> *val = (CValue<float>*) cap->getValue();

		if (val -> Get() != fpac) {
			val->Set(fpac);
			cap->Notify();
		}
	} else {
		LOGDEBUG(logger, "BUG: " << CAPA_INVERTER_ACPOWER_TOTAL
				<< " not a float ");
	}

	return true;
}

bool CInverterSputnikSSeries::token_KHR(const vector<string> & tokens)
{
	// Power-On-Hours
	if (tokens.size() != 2)
		return false;

	unsigned int tmp;
	float f;
	sscanf(tokens[1].c_str(), "%x", &tmp);

	f = tmp;

	// lookup if we already know that information.
	CCapability *cap = GetConcreteCapability(CAPA_INVERTER_PON_HOURS);

	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_PON_HOURS;
		v = IValue::Factory(CAPA_INVERTER_PON_HOURS_TYPE);
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if (cap->getValue()->GetType() == CAPA_INVERTER_PON_HOURS_TYPE) {
		CValue<float> *val = (CValue<float>*) cap->getValue();
		if (val -> Get() != f) {
			val->Set(f);
			cap->Notify();
		}
	} else {
		LOGDEBUG(logger, "BUG: " << CAPA_INVERTER_PON_HOURS << " not a float ");
	}

	return true;
}

bool CInverterSputnikSSeries::token_DYR(const vector<string> & tokens)
{
	// FIXME TODO Implement me!
	return true;
}

bool CInverterSputnikSSeries::token_DMT(const vector<string> & tokens)
{
	// FIXME TODO Implement me!
	return true;
}

bool CInverterSputnikSSeries::token_DDY(const vector<string> & tokens)
{
	// FIXME TODO Implement me!
	return true;
}

bool CInverterSputnikSSeries::token_KYR(const vector<string> & tokens)
{
	// Unit kwH
	if (tokens.size() != 2)
		return false;

	unsigned int raw;
	float kwh;
	sscanf(tokens[1].c_str(), "%x", &raw);

	kwh = raw;

	// lookup if we already know that information.
	CCapability *cap = GetConcreteCapability(CAPA_INVERTER_KWH_Y2D);

	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_KWH_Y2D;
		v = IValue::Factory(CAPA_INVERTER_KWH_Y2D_TYPE);
		((CValue<float>*) v)->Set(kwh);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if (cap->getValue()->GetType() == CAPA_INVERTER_KWH_Y2D_TYPE) {
		CValue<float> *val = (CValue<float>*) cap->getValue();

		if (val -> Get() != kwh) {
			val->Set(kwh);
			cap->Notify();
		}
	} else {
		LOGDEBUG(logger, "BUG: " << CAPA_INVERTER_KWH_Y2D << " not a float ");
	}

	return true;
}

bool CInverterSputnikSSeries::token_KMT(const vector<string> & tokens)
{
	// Unit kwH
	if (tokens.size() != 2)
		return false;

	unsigned int raw;
	float kwh;
	sscanf(tokens[1].c_str(), "%x", &raw);

	kwh = raw;

	// lookup if we already know that information.
	CCapability *cap = GetConcreteCapability(CAPA_INVERTER_KWH_M2D);

	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_KWH_M2D;
		v = IValue::Factory(CAPA_INVERTER_KWH_M2D_TYPE);
		((CValue<float>*) v)->Set(kwh);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if (cap->getValue()->GetType() == CAPA_INVERTER_KWH_M2D_TYPE) {
		CValue<float> *val = (CValue<float>*) cap->getValue();

		if (val -> Get() != kwh) {
			val->Set(kwh);
			cap->Notify();
		}
	} else {
		LOGDEBUG(logger, "BUG: " << CAPA_INVERTER_KWH_M2D << " not a float ");
	}

	return true;
}

bool CInverterSputnikSSeries::token_KDY(const vector<string> & tokens)
{
	// Unit 0.1 kwH
	if (tokens.size() != 2)
		return false;

	unsigned int raw;
	float kwh;
	sscanf(tokens[1].c_str(), "%x", &raw);

	kwh = raw / 10.0;

	// lookup if we already know that information.
	CCapability *cap = GetConcreteCapability(CAPA_INVERTER_KWH_2D);
	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_KWH_2D;
		v = IValue::Factory(CAPA_INVERTER_KWH_2D_TYPE);
		((CValue<float>*) v)->Set(kwh);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if (cap->getValue()->GetType() == CAPA_INVERTER_KWH_2D_TYPE) {
		CValue<float> *val = (CValue<float>*) cap->getValue();

		if (val -> Get() != kwh) {
			val->Set(kwh);
			cap->Notify();
		}
	} else {
		LOGDEBUG(logger, "BUG: " << CAPA_INVERTER_KWH_2D << " not a float ");
	}

	return true;
}

bool CInverterSputnikSSeries::token_KT0(const vector<string> & tokens)
{
	// FIXME TODO Implement me!
	// Unit kwH
	if (tokens.size() != 2)
		return false;

	unsigned int raw;
	float kwh;
	sscanf(tokens[1].c_str(), "%x", &raw);

	kwh = raw;

	// lookup if we already know that information.
	CCapability *cap = GetConcreteCapability(CAPA_INVERTER_KWH_TOTAL_NAME);

	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_KWH_TOTAL_NAME;
		v = IValue::Factory(CAPA_INVERTER_KWH_TOTAL_TYPE);
		((CValue<float>*) v)->Set(kwh);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if (cap->getValue()->GetType() == CAPA_INVERTER_KWH_TOTAL_TYPE) {
		CValue<float> *val = (CValue<float>*) cap->getValue();

		if (val -> Get() != kwh) {
			val->Set(kwh);
			cap->Notify();
		}
	} else {
		LOGDEBUG(logger, "BUG: " << CAPA_INVERTER_KWH_TOTAL_NAME
				<< " not a float ");
	}

	return true;
}

bool CInverterSputnikSSeries::token_PIN(const vector<string> & tokens)
{
	// Unit 0.5 Watts
	if (tokens.size() != 2)
		return false;

	unsigned int raw;
	float f;
	sscanf(tokens[1].c_str(), "%x", &raw);

	f = raw * 0.5;
	// lookup if we already know that information.
	CCapability *cap = GetConcreteCapability(CAPA_INVERTER_INSTALLEDPOWER_NAME);

	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_INSTALLEDPOWER_NAME;
		v = IValue::Factory(CAPA_INVERTER_INSTALLEDPOWER_TYPE);
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if (cap->getValue()->GetType() == CAPA_INVERTER_INSTALLEDPOWER_TYPE) {
		CValue<float> *val = (CValue<float>*) cap->getValue();

		if (val -> Get() != f) {
			val->Set(f);
			cap->Notify();
		}
	} else {
		LOGDEBUG(logger, "BUG: " << CAPA_INVERTER_INSTALLEDPOWER_NAME
				<< " not a float ");
	}

	return true;
}

bool CInverterSputnikSSeries::token_TNF(const vector<string> & tokens)
{
	// FIXME TODO Implement me!
	// Unit us
	// f = 1 / T , T = x / 1E6
	if (tokens.size() != 2)
		return false;

	unsigned int raw;
	float f;
	sscanf(tokens[1].c_str(), "%x", &raw);

	f = raw / 100.0;
	// lookup if we already know that information.
	CCapability *cap = GetConcreteCapability(CAPA_INVERTER_NET_FREQUENCY_NAME);

	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_NET_FREQUENCY_NAME;
		v = IValue::Factory(CAPA_INVERTER_NET_FREQUENCY_TYPE);
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if (cap->getValue()->GetType() == CAPA_INVERTER_NET_FREQUENCY_TYPE) {
		CValue<float> *val = (CValue<float>*) cap->getValue();

		if (val -> Get() != f) {
			val->Set(f);
			cap->Notify();
		}
	} else {
		LOGDEBUG(logger, "BUG: " << CAPA_INVERTER_NET_FREQUENCY_NAME
				<< " not a float ");
	}

	return true;
}

bool CInverterSputnikSSeries::token_PRL(const vector<string> & tokens)
{
	// Unit 1 %
	if (tokens.size() != 2)
		return false;

	unsigned int raw;
	float f;
	sscanf(tokens[1].c_str(), "%x", &raw);

	f = raw;
	// lookup if we already know that information.
	CCapability *cap = GetConcreteCapability(CAPA_INVERTER_RELPOWER_NAME);

	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_RELPOWER_NAME;
		v = IValue::Factory(CAPA_INVERTER_RELPOWER_TYPE);
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if (cap->getValue()->GetType() == CAPA_INVERTER_RELPOWER_TYPE) {
		CValue<float> *val = (CValue<float>*) cap->getValue();

		if (val -> Get() != f) {
			val->Set(f);
			cap->Notify();
		}
	} else {
		LOGDEBUG(logger, "BUG: " << CAPA_INVERTER_RELPOWER_NAME
				<< " not a float ");
	}

	return true;

	// FIXME TODO Implement me!

	return true;
}

bool CInverterSputnikSSeries::token_UDC(const vector<string> & tokens)
{
	// Unit 0.1 Volts
	if (tokens.size() != 2)
		return false;

	unsigned int raw;
	float f;
	sscanf(tokens[1].c_str(), "%x", &raw);

	f = raw * 0.1;
	// lookup if we already know that information.
	CCapability *cap = GetConcreteCapability(
			CAPA_INVERTER_INPUT_DC_VOLTAGE_NAME);

	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_INPUT_DC_VOLTAGE_NAME;
		v = IValue::Factory(CAPA_INVERTER_INPUT_DC_VOLTAGE_TYPE);
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if (cap->getValue()->GetType() == CAPA_INVERTER_INPUT_DC_VOLTAGE_TYPE) {
		CValue<float> *val = (CValue<float>*) cap->getValue();

		if (val -> Get() != f) {
			val->Set(f);
			cap->Notify();
		}
	} else {
		LOGDEBUG(logger, "BUG: " << CAPA_INVERTER_INPUT_DC_VOLTAGE_NAME
				<< " not a float ");
	}

	return true;
}

bool CInverterSputnikSSeries::token_UL1(const vector<string> & tokens)
{ // Unit 0.1 Volts
	if (tokens.size() != 2)
		return false;

	unsigned int raw;
	float f;
	sscanf(tokens[1].c_str(), "%x", &raw);

	f = raw * 0.1;
	// lookup if we already know that information.
	CCapability *cap =
			GetConcreteCapability(CAPA_INVERTER_GRID_AC_VOLTAGE_NAME);

	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_GRID_AC_VOLTAGE_NAME;
		v = IValue::Factory(CAPA_INVERTER_GRID_AC_VOLTAGE_TYPE);
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if (cap->getValue()->GetType() == CAPA_INVERTER_GRID_AC_VOLTAGE_TYPE) {
		CValue<float> *val = (CValue<float>*) cap->getValue();

		if (val -> Get() != f) {
			val->Set(f);
			cap->Notify();
		}
	} else {
		LOGDEBUG(logger, "BUG: " << CAPA_INVERTER_GRID_AC_VOLTAGE_NAME
				<< " not a float");
	}

	return true;
}

// TODO generate a more fitting concept for multi-phase invertes
// like "pseudo-inverters"
bool CInverterSputnikSSeries::token_UL2(const vector<string> & tokens)
{
	// FIXME TODO Implement me!
	return true;
}

bool CInverterSputnikSSeries::token_UL3(const vector<string> & tokens)
{
	// FIXME TODO Implement me!
	return true;
}

bool CInverterSputnikSSeries::token_IDC(const vector<string> & tokens)
{ // Unit 0.01 Amps
	if (tokens.size() != 2)
		return false;

	unsigned int raw;
	float f;
	sscanf(tokens[1].c_str(), "%x", &raw);

	f = raw * 0.01;
	// lookup if we already know that information.
	CCapability *cap = GetConcreteCapability(
			CAPA_INVERTER_INPUT_DC_CURRENT_NAME);

	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_INPUT_DC_CURRENT_NAME;
		v = IValue::Factory(CAPA_INVERTER_INPUT_DC_CURRENT_TYPE);
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if (cap->getValue()->GetType() == CAPA_INVERTER_INPUT_DC_CURRENT_TYPE) {
		CValue<float> *val = (CValue<float>*) cap->getValue();

		if (val -> Get() != f) {
			val->Set(f);
			cap->Notify();
		}
	} else {
		LOGDEBUG(logger, "BUG: " << CAPA_INVERTER_INPUT_DC_CURRENT_NAME
				<< " not a float");
	}

	return true;
}

bool CInverterSputnikSSeries::token_IL1(const vector<string> & tokens)
{
	// unit: 0.01 Amps
	if (tokens.size() != 2)
		return false;

	unsigned int raw;
	float f;
	sscanf(tokens[1].c_str(), "%x", &raw);

	f = raw * 0.01;
	// lookup if we already know that information.
	CCapability *cap =
			GetConcreteCapability(CAPA_INVERTER_GRID_AC_CURRENT_NAME);

	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_GRID_AC_CURRENT_NAME;
		v = IValue::Factory(CAPA_INVERTER_GRID_AC_CURRENT_TYPE);
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if (cap->getValue()->GetType() == CAPA_INVERTER_GRID_AC_CURRENT_TYPE) {
		CValue<float> *val = (CValue<float>*) cap->getValue();

		if (val -> Get() != f) {
			val->Set(f);
			cap->Notify();
		}
	} else {
		LOGDEBUG(logger, "BUG: " << CAPA_INVERTER_GRID_AC_CURRENT_NAME
				<< " not a float");
	}

	return true;
}

bool CInverterSputnikSSeries::token_IL2(const vector<string> & tokens)
{
	// FIXME TODO Implement me!
	return true;
}

bool CInverterSputnikSSeries::token_IL3(const vector<string> & tokens)
{
	// FIXME TODO Implement me!
	return true;
}

bool CInverterSputnikSSeries::token_TKK(const vector<string> & tokens)
{
	// Unit 1 °C
	if (tokens.size() != 2)
		return false;

	unsigned int raw;
	float f;
	sscanf(tokens[1].c_str(), "%x", &raw);

	f = raw;
	// lookup if we already know that information.
	CCapability *cap = GetConcreteCapability(CAPA_INVERTER_TEMPERATURE_NAME);

	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_TEMPERATURE_NAME;
		v = IValue::Factory(CAPA_INVERTER_TEMPERATURE_TYPE);
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if (cap->getValue()->GetType() == CAPA_INVERTER_TEMPERATURE_TYPE) {
		CValue<float> *val = (CValue<float>*) cap->getValue();

		if (val -> Get() != f) {
			val->Set(f);
			cap->Notify();
		}
	} else {
		LOGDEBUG(logger, "BUG: " << CAPA_INVERTER_TEMPERATURE_NAME
				<< " not a float");
	}

	return true;
}

bool CInverterSputnikSSeries::token_TK2(const vector<string> & tokens)
{
	// FIXME TODO Implement me!
	return true;
}

bool CInverterSputnikSSeries::token_TK3(const vector<string> & tokens)
{
	// FIXME TODO Implement me!
	return true;
}

bool CInverterSputnikSSeries::token_TMI(const vector<string> & tokens)
{
	// FIXME TODO Implement me!
	return true;
}

bool CInverterSputnikSSeries::token_THR(const vector<string> & tokens)
{
	// FIXME TODO Implement me!
	return true;
}

bool CInverterSputnikSSeries::token_SYS(const vector<string> &tokens)
{
	// gets the system state of the inverter.
	// note: Alarms are handled with another command, SAL.

	// SYS reponses a code (eg. 20004) and a second parameter, which I
	// never saw != 0.
	if (tokens.size() != 3)
		return false;

	if (tokens[2] != "0") {
		LOGINFO(logger, "Received an unknown SYS response. Please file a bug"
				<< " along with the following: " << tokens[0] << ","
				<< tokens[1] << "," << tokens[2]);
	}

	unsigned int code;
	sscanf(tokens[1].c_str(), "%x", &code);

	string description;

	int i = 0;
	do {
		if (statuscodes[i].code == code)
			break;
	} while (statuscodes[++i].code != (unsigned int) -1);

	if (laststatuscode != (unsigned int) -1 && statuscodes[i].code
			== (unsigned int) -1) {
		LOGINFO(logger, "SYS reported an (too us) unknown status code of "
				<< tokens[0] << "=" << tokens[1] << "," << tokens[2]
		);
		LOGINFO (logger,
				" PLEASE file a with all information you have, for example,"
				<< " reading the display of the inverter and of course the infors given above."
		);
	}

	laststatuscode = statuscodes[i].code;

	/* Update the status */
	CCapability *cap = GetConcreteCapability(CAPA_INVERTER_STATUS_NAME);
	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_STATUS_NAME;
		v = IValue::Factory(CAPA_INVERTER_STATUS_TYPE);
		((CValue<int>*) v)->Set(statuscodes[i].status);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	} else if (cap->getValue()->GetType() == CAPA_INVERTER_STATUS_TYPE) {
		CValue<int> * val = (CValue<int> *) cap->getValue();
		if (val->Get() != statuscodes[i].status) {
			val->Set(statuscodes[i].status);
			cap->Notify();
		}
	} else {
		LOGDEBUG(logger, "BUG: " << CAPA_INVERTER_STATUS_NAME << " not a int");
	}

	// now also do the same with the string.
	cap = GetConcreteCapability(CAPA_INVERTER_STATUS_READABLE_NAME);
	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_STATUS_READABLE_NAME;
		v = IValue::Factory(CAPA_INVERTER_STATUS_READABLE_TYPE);
		((CValue<string>*) v)->Set(statuscodes[i].description);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	} else if (cap->getValue()->GetType() == CAPA_INVERTER_STATUS_READABLE_TYPE) {
		CValue<string> * val = (CValue<string> *) cap->getValue();
		if (val->Get() != statuscodes[i].description) {
			val->Set(statuscodes[i].description);
			cap->Notify();
		}
	} else {
		LOGDEBUG(logger, "BUG: " << CAPA_INVERTER_STATUS_READABLE_NAME
				<< " not a string ");
	}

	return true;
}

/** helper that builds the firmware version capability.
 *
 * Will only build the data, if at least the main version (major version)
 * is known.
 * Adds the build version if available.
 *
 * Note: This should only be called if any change is detected!
 */
void CInverterSputnikSSeries::create_versioncapa(void)
{
	string ver;
	char buf[128];

	unsigned int major, minor;

	// Won't build a version string if the "major" version is unknown.
	if (!swversion)
		return;

	major = swversion / 10;
	minor = swversion % 10;

	if (swbuild) {
		sprintf(buf, "%d.%d Build %d", major, minor, swbuild);
	} else {
		sprintf(buf, "%d.%d", major, minor);
	}

	ver = buf;

	// lookup if we already know that information.
	CCapability *cap = GetConcreteCapability(CAPA_INVERTER_FIRMWARE);

	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_FIRMWARE;
		v = IValue::Factory(CAPA_INVERTER_FIRMWARE_TYPE);
		((CValue<string>*) v)->Set(ver);
		c = new CCapability(s, v, this);
		AddCapability(c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	} else if (cap->getValue()->GetType() == IValue::string_type) {
		CValue<string> *val = (CValue<string>*) cap->getValue();
		if (ver != val->Get()) {
			val->Set(ver);
			cap->Notify();
		}
	}
}

#endif
