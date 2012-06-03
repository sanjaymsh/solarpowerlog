/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2009-2012 Tobias Frost

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
 *
 *  Author: Tobias Frost
 *
 *  Contributors:
 *      E.A.Neonakis <eaneonakis@freemail.gr>
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

#include "Inverters/SputnikEngineering/SputnikCommand/CSputnikCommand.h"
#include "Inverters/SputnikEngineering/SputnikCommand/CSputnikCommandSoftwareVersion.h"
#include "Inverters/SputnikEngineering/SputnikCommand/CSputnikCommandSYS.h"
#include "Inverters/SputnikEngineering/SputnikCommand/CSputnikCommandTYP.h"
#include "Inverters/SputnikEngineering/SputnikCommand/BackoffStrategies/CSputnikCmdBOOnce.h"
#include "Inverters/SputnikEngineering/SputnikCommand/BackoffStrategies/CSputnikCmdBOTimed.h"
#include "Inverters/SputnikEngineering/SputnikCommand/BackoffStrategies/CSputnikCmdBOIfSupported.h"

using namespace libconfig;

using namespace std;

#undef DEBUG_TOKENIZER
// Debug: Print all received tokens
#if defined DEBUG_TOKENIZER
void DEBUG_tokenprinter(ILogger &logger, std::vector<std::string> tokens) {
    int i;
    if (logger.IsEnabled(ILogger::LL_TRACE)) {
        std::stringstream ss;
        vector<string>::iterator it;
        for (i = 0, it = tokens.begin(); it != tokens.end() - 1; it++) {
            ss << i++ << ": " << (*it) << "\tlen: " << (*it).length() << endl;
        }
        LOGTRACE(logger, ss);
    }

}
#endif

CInverterSputnikSSeries::CInverterSputnikSSeries(const string &name,
		const string & configurationpath) :
	IInverterBase::IInverterBase(name, configurationpath, "inverter")
{
	ownadr = 0xfb;
	commadr = 0x01;

	// Add the capabilites that this inverter has
	// Note: The "must-have" ones CAPA_CAPAS_REMOVEALL and CAPA_CAPAS_UPDATED are already instanciated by the base class constructor.
	// Note2: You also can add capabilites as soon you know them (runtime detection)

	string s;
	IValue *v;
	CCapability *c;
	s = CAPA_INVERTER_MANUFACTOR_NAME;
	v = CValueFactory::Factory<CAPA_INVERTER_MANUFACTOR_TYPE>();
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
	v = CValueFactory::Factory<CAPA_INVERTER_QUERYINTERVAL_TYPE>();
	((CValue<float>*) v)->Set(interval);
	c = new CCapability(s, v, this);
	AddCapability(c);

	s = CAPA_INVERTER_CONFIGNAME;
	v = CValueFactory::Factory<CAPA_INVERTER_CONFIGNAME_TYPE>();
	((CValue<std::string>*) v)->Set(name);
	c = new CCapability(s, v, this);
	AddCapability(c);

	LOGDEBUG(logger,"Inverter configuration:");
	LOGDEBUG(logger,"class CInverterSputnikSSeries ");
	LOGDEBUG(logger,"Query Interval: "<< interval);
	LOGDEBUG(logger,"Ownadr: " << ownadr << " Commadr: " << commadr);
	cfghlp.GetConfig("comms", s, (string) "unset");
	LOGDEBUG(logger,"Communication: " << s);

	// Initialize vector of supported commands.
	// Handles the "TYP" command, which will identifiy the model
	// and handles CAPA_INVERTER_MODEL.
    commands.push_back(new CSputnikCommandTYP(this, new CSputnikCmdBOOnce));

    // Gets SW version
    commands.push_back(
        new CSputnikCommandSoftwareVersion(this, CAPA_INVERTER_FIRMWARE,new CSputnikCmdBOOnce));

//	needs special handling this->commands.push_back(CSputnikCommand<unsigned long>("EC*",27,0));

    // AC Power
    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_ACPOWER_TOTAL_TYPE>("PAC", 9, 0.5,
            this, CAPA_INVERTER_ACPOWER_TOTAL));

    // DC Power
    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_DCPOWER_TOTAL_TYPE>("PDC", 9, 0.5,
            this, CAPA_INVERTER_DCPOWER_TOTAL));

    // On Time inverter in hours.
    boost::posix_time::time_duration time_between(boost::posix_time::hours(0)+boost::posix_time::minutes(1));;
    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_PON_HOURS_TYPE>("KHR", 9, 1.0, this,
            CAPA_INVERTER_PON_HOURS,
            new CSputnikCmdBOTimed(time_between)));

    // Number of startups -- only once per conncetion.
    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_STARTUPS_TYPE>("CAC", 9, 1.0, this,
            CAPA_INVERTER_STARTUPS, new CSputnikCmdBOOnce));

// not implemented  this->commands.push_back(CSputnikCommand<unsigned long>("DYR",7,0));
// not implemented	this->commands.push_back(CSputnikCommand<unsigned long>("DDY",7,0));
// not implemented  this->commands.push_back(CSputnikCommand<unsigned long>("DMT",7,0));

    // Number of kwH this year.
    // as the unit is one kWh, we can reduce the query time to e.g two times a minute
    // (which would need 120kW feeding power ;-)
    time_between = boost::posix_time::seconds(30);
    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_KWH_Y2D_TYPE>("KYR", 9, 1.0, this,
            CAPA_INVERTER_KWH_Y2D, new CSputnikCmdBOTimed(time_between)));

    // Number of kwH today. Same logic as for KYR, limiting to 2 times a minute.
    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_KWH_M2D_TYPE>("KMT", 7, 1.0, this,
            CAPA_INVERTER_KWH_M2D, new CSputnikCmdBOTimed(time_between)));

    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_KWH_2D_TYPE>("KDY", 10, 0.1, this,
            CAPA_INVERTER_KWH_2D));

    // kwH produced yesterday.
    // Only once a session.
    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_KWH_YD_TYPE>("KLD", 10, 0.1, this,
            CAPA_INVERTER_KWH_YD, new CSputnikCmdBOOnce));

    // All-time kwH
    // also only 2 times a minute.
    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_KWH_TOTAL_TYPE>("KT0", 10, 1.0, this,
            CAPA_INVERTER_KWH_TOTAL_NAME,
            new CSputnikCmdBOTimed(time_between)));

    // Installed power .. will not change over a session.
    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_INSTALLEDPOWER_TYPE>("PIN", 9, 0.5,
            this, CAPA_INVERTER_INSTALLEDPOWER_NAME, new CSputnikCmdBOOnce));

    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_NET_FREQUENCY_TYPE>("TNF", 10, 0.01,
            this, CAPA_INVERTER_NET_FREQUENCY_NAME));

    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_RELPOWER_TYPE>("PRL", 10, 1.0, this,
            CAPA_INVERTER_RELPOWER_NAME));

    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_INPUT_DC_VOLTAGE_TYPE>("UDC", 10, 0.1,
            this, CAPA_INVERTER_INPUT_DC_VOLTAGE_NAME));

    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_GRID_AC_VOLTAGE_TYPE>("UL1", 10, 0.1,
            this, CAPA_INVERTER_GRID_AC_VOLTAGE_NAME));

    // First, implement the "this command is not supported" scheme.
    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_GRID_AC_VOLTAGE_PHASE2_TYPE>("UL2",
            10, 0.1, this, CAPA_INVERTER_GRID_AC_VOLTAGE_PHASE2_NAME,
            new CSputnikCmdBOIfSupported));

    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_GRID_AC_VOLTAGE_PHASE3_TYPE>("UL3",
            10, 0.1, this, CAPA_INVERTER_GRID_AC_VOLTAGE_PHASE3_NAME,
            new CSputnikCmdBOIfSupported));

    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_INPUT_DC_CURRENT_TYPE>("IDC", 10,
            0.01, this, CAPA_INVERTER_INPUT_DC_CURRENT_NAME));

    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_GRID_AC_CURRENT_TYPE>("IL1", 10, 0.01,
            this, CAPA_INVERTER_GRID_AC_CURRENT_NAME));

    // First, implement the "this command is not supported" scheme.
    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_GRID_AC_CURRENT_PHASE2_TYPE>("IL2", 10, 0.1,
            this, CAPA_INVERTER_GRID_AC_CURRENT_PHASE2_NAME, new CSputnikCmdBOIfSupported));

    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_GRID_AC_CURRENT_PHASE3_TYPE>("IL3", 10, 0.1,
            this, CAPA_INVERTER_GRID_AC_CURRENT_PHASE3_NAME, new CSputnikCmdBOIfSupported));

    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_TEMPERATURE_TYPE>("TKK", 10, 1.0,
            this, CAPA_INVERTER_TEMPERATURE_NAME));

    // First, implement the "this command is not supported" scheme.
    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_TEMPERATURE_PHASE2_TYPE>("TK2",10,1.0, this,
            CAPA_INVERTER_TEMPERATURE_PHASE2_NAME, new CSputnikCmdBOIfSupported));

    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_TEMPERATURE_PHASE3_TYPE>("TK3",10,1.0, this,
            CAPA_INVERTER_TEMPERATURE_PHASE3_NAME, new CSputnikCmdBOIfSupported));

    // Get Inverter Timestamp / (Hour:Minute)
    // Should be s special implementation!
    // But the clock is rather inaccurate, so the compuiter's timestamp is
    // far more precice.
    // Note: It is possible to set the time/hour via the interface, but
    // not implemented
    // this->commands.push_back(CSputnikCommand<unsigned long>("TMI",10,0));
    // not implemented
    // this->commands.push_back(CSputnikCommand<unsigned long>("THR",10,0));

    // Handles the SYS Command, which handles the CAPA_INVERTER_STATUS_NAME
    // and CAPA_INVERTER_STATUS_READABLE_NAME capabilities.
    commands.push_back(new CSputnikCommandSYS(this));

    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_ERROR_CURRENT_TYPE>("IEE", 10, 0.1,
            this, CAPA_INVERTER_ERROR_CURRENT_NAME));
    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_DC_ERROR_CURRENT_TYPE>("IED", 10, 0.1,
            this, CAPA_INVERTER_DC_ERROR_CURRENT_NAME));
    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_AC_ERROR_CURRENT_TYPE>("IEA", 10, 0.1,
            this, CAPA_INVERTER_AC_ERROR_CURRENT_NAME));
    commands.push_back(
        new CSputnikCommand<CAPA_INVERTER_GROUND_VOLTAGE_TYPE>("UGD", 10, 0.1,
            this, CAPA_INVERTER_GROUND_VOLTAGE_NAME));

}

CInverterSputnikSSeries::~CInverterSputnikSSeries()
{
    /* delete all commands allocated in the constructor. */
	vector<ISputnikCommand*>::iterator it;
	for (it=commands.begin(); it!=commands.end(); it++) {
	    delete *it;
	    *it= NULL;
	}
	commands.clear();
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
 * telegram, and only the checksum and the } is missing.
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

		// reset the backoff algorithms for the commands.
		this->pendingcommands.clear();
		this->notansweredcommands.clear();
		vector<ISputnikCommand *>::iterator it;
		for (it=this->commands.begin(); it!=commands.end(); it++) {
		    (*it)->InverterDisconnected();
		}

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

	case CMD_QUERY_POLL:
	{
		LOGDEBUG(logger, "new state: CMD_QUERY_POLL ");

		// Collect all queries to be issued.
		std::vector<ISputnikCommand*>::iterator it;
		for (it=commands.begin(); it!= commands.end(); it++) {
		    if ((*it)->ConsiderCommand()) {
		        pendingcommands.push_back(*it);
		    }
		}
	}

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
			// we do not differentiate the error here, an error is an error....
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

        // all issued commands should have been answered,
        // those in the notanswered set, have not been answered and we notfify
        // the commands to pass that information to their backoff algorithms.
        std::set<ISputnikCommand*>::iterator it;
        for (it=notansweredcommands.begin();it!=notansweredcommands.end();it++) {
            (*it)->CommandNotAnswered();
        }
        notansweredcommands.clear();

		// if there are still pending commands, issue them first before
		// filling the queue again.
		if (!pendingcommands.empty()) {
			cmd = new ICommand(CMD_SEND_QUERIES, this);
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

    // ensure two things:
    // - telegram len does not exceed 255 bytes in total,
    // while there are 16 header bytes and 6 trailing bytes to be considered.
    // - answer is not exceeding 255 bytes
    // (here, we reserve a saftey of 10 bytes additionally).
    int telegramlen = 254-22;
    int expectedanswerlen = 255-31;
    int currentport = QUERY; // At the moment only QUERY's are supported.
    std::string telegram;

    if (pendingcommands.empty()) return "";
    // assemble string to send out of pending commands.

    // get the max amount of commands up to the max size
    // (we also ensure max answer len, as on observations fragmentation of
    // the telgramm does break it -- at least on my inverters' firmware.
    std::vector<ISputnikCommand*>::iterator it = pendingcommands.begin();
    while (it != pendingcommands.end()) {
        int alen = (*it)->GetMaxAnswerLen();
        int clen = (*it)->GetCommandLen();
        if ( alen < expectedanswerlen && clen < telegramlen ) {
            if (!telegram.empty()) {
                // Add seperator if this is not the first command in the string.
                telegram += ";";
                telegramlen--;
            }
            telegram += (*it)->GetCommand();
            telegramlen -= clen;
            expectedanswerlen -=alen;
            notansweredcommands.insert(*it);
            it = pendingcommands.erase(it);
        }
        else
        {
            it++;
        }
    }

    int len = 0;
    char buf[32];
    snprintf(buf, 32,"%X:", currentport);
    len = strlen(buf) + telegram.length() + 10 + 6;
    snprintf(buf, 32, "{%02X;%02X;%02X|%X:", ownadr, commadr, len,
            currentport);
    // Insert header at beginning, add trailing "|"
    telegram.insert(0,buf);
    telegram.append("|");
    snprintf(buf,32,"%04X}", CalcChecksum(telegram.c_str(),
        telegram.length()));
    telegram.append(buf);
    return telegram;
}

int CInverterSputnikSSeries::parsereceivedstring(const string & s) {

    unsigned int i;

    // check for basic constraints...
    if (s[0] != '{' || s[s.length() - 1] != '}') return -1;
    // tokenizer (taken from
    // http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html
    // but  modified.

    // we take the received string and "split" it into smaller pieces.
    // the pieces ("tokens") then assemble one single information to be
    // for this, we split by:
    // ";" "|" ":" (and "{}")

    vector<string> tokens;
    {
        char delimiters[] = "{;|:}";
        tokenizer(delimiters, s, tokens);
    }
    // Debug: Print all received tokens
#if defined DEBUG_TOKENIZER
    DEBUG_tokenprinter(this->logger, tokens);
#endif

    unsigned int tmp;
    if (1 != sscanf(tokens.back().c_str(), "%x", &tmp)) {
        LOGDEBUG(logger, "could not parse checksum. Token was:");
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

    {
        int ret = 1;
        const char delimiters[] = "=,";
        for (i = 4; i < tokens.size() - 1; i++) {
            vector<std::string> subtokens;
            tokenizer(delimiters, tokens[i], subtokens);
            if (subtokens.empty()) continue;
            vector<ISputnikCommand*>::iterator it;
            for (it = commands.begin(); it != commands.end(); it++) {
                if ((*it)->IsHandled(subtokens[0])) {
                    LOGTRACE(logger,"Now handling: " << tokens[i] << " ret=" << ret);
                    bool result = (*it)->handle_token(subtokens);
                    if (!result)  {
                        LOGTRACE(logger,"failed parsing " + tokens[i]);
                        ret = -1;
                    }
                    else {
                        notansweredcommands.erase(*it);
                    }
                    break;
                }
            }
        }
        return ret;
    }
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

#endif
