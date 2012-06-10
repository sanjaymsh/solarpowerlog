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

/** \file CInverterSputnikSSeriesSimulator.cpp
 *
 *  Created on: June 07, 2012
 *
 *  Author: Tobias Frost
 *
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_INV_SPUTNIKSIMULATOR

#include "configuration/Registry.h"
#include "configuration/CConfigHelper.h"

#include "CInverterSputnikSSeriesSimulator.h"

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


struct CInverterSputnikSSeriesSimulator::simulator_commands simcommands[] = {
        { "PAC", 0.5, new CValue<float>(42), 0, NULL },
        { "PDC", 0.5, new CValue<float>(42), 0, NULL },
        { "KHR", 1.0, new CValue<float>(42), 0, NULL },
        { "CAC", 1.0, new CValue<long>(42), 0, NULL },
        { "SYS", 1.0, new CValue<int>(20004), 1, new CValue<int>(0) },
        { "TYP", 1.0, new CValue<int>(65534), 0, NULL },
        { "BDN", 1.0, new CValue<int>(24), 0, NULL },
        { "SWV", 1.0, new CValue<int>(0), 0, NULL },
        { "KYR", 1.0, new CValue<float>(42), 0, NULL },
        { "KMT", 1.0, new CValue<float>(42), 0, NULL },
        { "KDY", 0.1, new CValue<float>(42), 0, NULL },
        { "KLD", 0.1, new CValue<float>(42), 0, NULL },
        { "KT0", 1.0, new CValue<float>(42), 0, NULL },
        { "PIN", 0.5, new CValue<float>(42), 0, NULL },
        { "TNF", 0.01, new CValue<float>(50), 0, NULL },
        { "PRL", 1.0, new CValue<float>(42), 0, NULL },
        { "PRL", 1.0, new CValue<float>(42), 0, NULL },
        { "UDC", 0.1, new CValue<float>(200), 0, NULL },
        { "UL1", 0.1, new CValue<float>(230), 0, NULL },
        { "UL2", 0.1, new CValue<float>(200), 0, NULL },
        { "UL3", 0.1, new CValue<float>(230), 0, NULL },
        { "IDC", 0.01, new CValue<float>(2), 0, NULL },
        { "IL1", 0.01, new CValue<float>(2), 0, NULL },
        { "IL2", 0.01, new CValue<float>(2), 0, NULL },
        { "IL3", 0.01, new CValue<float>(2), 0, NULL },
        { "TKK", 1.0, new CValue<float>(42), 0, NULL },
        { "TK2", 1.0, new CValue<float>(42), 0, NULL },
        { "TK3", 1.0, new CValue<float>(42), 0, NULL },
        { "IEE", 0.1, new CValue<float>(1), 0, NULL },
        { "IED", 0.1, new CValue<float>(1), 0, NULL },
        { "IEA", 0.1, new CValue<float>(1), 0, NULL },
        { "UGD", 0.1, new CValue<float>(25), 0, NULL },
{ NULL , 0  , NULL, 0, NULL}
};

// helper to convert the values to suitable strings
// Implemented for float, long and int
static std::string convert2sputnikhex(IValue *value, float scale) {
    char buf[32];
    unsigned long tmp;
    if (CValue<float>::IsType(value)) {
        float ftmp = ((CValue<float>*) value)->Get() / scale + 0.5;
        tmp = (long) ftmp;
    } else if (CValue<long>::IsType(value)) {
        float ftmp = ((CValue<long>*) value)->Get() / scale + 0.5;
        tmp = (long) ftmp;
    } else if (CValue<int>::IsType(value)) {
        float ftmp = ((CValue<int>*) value)->Get() / scale + 0.5;
        tmp = (long) ftmp;
    } else if (CValue<int>::IsType(value)) {
        bool btmp = ((CValue<bool>*) value)->Get();
        tmp = (long) btmp;
    } else {
        LOGERROR(Registry::GetMainLogger(),"convert2sputnikhex -- unknown CValue<type>");
        return "";
    }

    snprintf(buf,sizeof(buf),"%lX",tmp);
    return buf;
}


CInverterSputnikSSeriesSimulator::CInverterSputnikSSeriesSimulator(const string &name,
		const string & configurationpath) :
	IInverterBase::IInverterBase(name, configurationpath, "inverter")
{
	commadr = 0x01;

	// Add the capabilites that this inverter has
	// Note: The "must-have" ones CAPA_CAPAS_REMOVEALL and CAPA_CAPAS_UPDATED are already instanciated by the base class constructor.
	// Note2: You also can add capabilites as soon you know them (runtime detection)

	string s;
	IValue *v;
	CCapability *c;
	s = CAPA_INVERTER_MANUFACTOR_NAME;
	v = CValueFactory::Factory<CAPA_INVERTER_MANUFACTOR_TYPE>();
	((CValue<string>*) v)->Set("Solarpowerlog");
	c = new CCapability(s, v, this);
	AddCapability(c);

	// Add the request to initialize as soon as the system is up.
	ICommand *cmd = new ICommand(CMD_INIT, this);
	Registry::GetMainScheduler()->ScheduleWork(cmd);

	CConfigHelper cfghlp(configurationpath);

	// Query settings needed and default all optional settings.
	cfghlp.GetConfig("commadr", commadr, 0x01u);

	s = CAPA_INVERTER_CONFIGNAME;
	v = CValueFactory::Factory<CAPA_INVERTER_CONFIGNAME_TYPE>();
	((CValue<std::string>*) v)->Set(name);
	c = new CCapability(s, v, this);
	AddCapability(c);

	LOGDEBUG(logger,"Inverter configuration:");
	LOGDEBUG(logger,"class CInverterSputnikSSeriesSimulator ");
	LOGDEBUG(logger,"Commadr: " << commadr);
	cfghlp.GetConfig("comms", s, (string) "unset");
	LOGDEBUG(logger,"Communication: " << s);

	this->scommands = new struct simulator_commands[sizeof(simcommands)
            / sizeof(struct simulator_commands)];

	int i=0;
	do {
	    scommands[i].token  = simcommands[i].token;
	    scommands[i].scale1 = simcommands[i].scale1;
	    scommands[i].scale2 = simcommands[i].scale2;
	    scommands[i].value = NULL;
	    scommands[i].value2 = NULL;
	    if (simcommands[i].value) scommands[i].value  = simcommands[i].value->clone();
	    if (simcommands[i].value2) scommands[i].value2  = simcommands[i].value2->clone();
	    LOGTRACE( logger,
                "scommmand["<<i<<"]:"
                " token=" << scommands[i].token <<
                " value="<< (void*)scommands[i].value <<
                " value2=" << (void*)scommands[i].value2);
	} while (simcommands[i++].token);
}

CInverterSputnikSSeriesSimulator::~CInverterSputnikSSeriesSimulator()
{
    int i=0;
    do {
       if (scommands[i].value) delete scommands[i].value;
       if (scommands[i].value2) delete scommands[i].value2;
    } while (scommands[++i].token);
    delete[] scommands;
}

bool CInverterSputnikSSeriesSimulator::CheckConfig()
{
	string setting;
	string str;

	bool fail = false;

	CConfigHelper hlp(configurationpath);
	fail |= (true != hlp.CheckConfig("comms", libconfig::Setting::TypeString));
	// Note: Queryinterval is optional. But CConfigHelper handle also opt.
	// parameters and checks for type.
	fail |= (true != hlp.CheckConfig("queryinterval",
	        libconfig::Setting::TypeFloat, true));
	fail |= (true != hlp.CheckConfig("commadr", libconfig::Setting::TypeInt));

	// Check config of the component, if already instanciated.

	if (connection) {
		fail |= (true != connection->CheckConfig());
	}

	if ( ! connection->CanAccept()) {
	    LOGFATAL(logger,"Configuration Error: Communication method supports not "
	            "server-mode or is not configured accordingly.");
	    fail = true;
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
unsigned int CInverterSputnikSSeriesSimulator::CalcChecksum(const char *str, int len)
{
	unsigned int chksum = 0;
	str++;
	do {
		chksum += *str++;
	} while (--len);

	return chksum;
}

void CInverterSputnikSSeriesSimulator::ExecuteCommand(const ICommand *Command)
{
	string commstring = "";
	string reccomm = "";
	ICommand *cmd;

	switch ((Commands) Command->getCmd())
	{

	case CMD_INIT:
	{
		LOGDEBUG(logger, "new state: CMD_INIT");

		cmd = new ICommand(CMD_CONNECTED, this);
		if (connection->IsConnected()) connection->Disconnect(NULL);
		connection->Accept(cmd);
		break;

		// storage of objects in boost::any
		//cmd->addData("TEST", cmd);
		//cmd = boost::any_cast<ICommand*>(cmd->findData("TEST"));
	}

	case CMD_CONNECTED:
	{
		LOGDEBUG(logger, "new state: CMD_CONNECTED");

		int err = -1;
		// CMD_CONNECTED: Accept succeeded of failure.
		try {
			err = boost::any_cast<int>(Command->findData(ICMD_ERRNO));
		} catch (...) {
			LOGDEBUG(logger,"CMD_CONNECTED: unexpected exception while trying to get errorcode.");
			err = -1;
		}

		if (err < 0) {
			try {
				LOGERROR(logger, "Error while connecting: " <<
						boost::any_cast<string>(Command->findData(ICMD_ERRNO_STR)));
			} catch (...) {
				LOGERROR(logger, "Unknown error " << err << " while connecting.");
			}

			cmd = new ICommand(CMD_INIT, this);
	        timespec ts;
	        ts.tv_sec = 15;
	        ts.tv_nsec = 0;
			Registry::GetMainScheduler()->ScheduleWork(cmd, ts);
			break;
		} else {
	        cmd = new ICommand(CMD_EVALUATE_RECEIVE, this);
	        cmd->addData(ICONN_TOKEN_TIMEOUT,(unsigned long)3600*1000);
	        connection->Receive(cmd);
		}
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
			cmd = new ICommand(CMD_INIT, this);
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

		LOGTRACE(logger, "Received: " << s << " len: " << s.size());
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

		// make answer and send it.
		s = parsereceivedstring(s);
        LOGTRACE(logger, "Response :" << s << "len: " << s.size());
		cmd = new ICommand(CMD_WAIT_SENT,this);
		cmd->addData(ICONN_TOKEN_SEND_STRING, s);
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
            LOGERROR(logger,"Error while sending");
            cmd = new ICommand(CMD_INIT, this);
            Registry::GetMainScheduler()->ScheduleWork(cmd);
            break;
        }

        cmd = new ICommand(CMD_EVALUATE_RECEIVE, this);
        cmd->addData(ICONN_TOKEN_TIMEOUT, (unsigned long) 3600 * 1000);
        connection->Receive(cmd);
        break;
        }

    // Fall through ok.
	default:
		LOGFATAL(logger, "Unknown CMD received");
		abort();
		break; // to have one code-analysis warning less.
	}

}

std::string CInverterSputnikSSeriesSimulator::parsereceivedstring(const string & s) {

    unsigned int i;
    unsigned int sender_adr;

    // check for basic constraints...
    if (s[0] != '{' || s[s.length() - 1] != '}') return "";
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

    // Minimum is {<from>;<to>;<len>|<port>: .... |<chksum>} - 5 tokens
    if (tokens.size() <= 5 ) return "";

    // Debug: Print all received tokens
#if defined DEBUG_TOKENIZER
    DEBUG_tokenprinter(this->logger, tokens);
#endif

    unsigned int tmp;
    if (1 != sscanf(tokens.back().c_str(), "%x", &tmp)) {
        LOGDEBUG(logger, "could not parse checksum. Token was:");
        return "";
    }

    if (tmp != CalcChecksum(s.c_str(), s.length() - 6)) {
        LOGDEBUG(logger, "Checksum error on received telegram");
        return "";
    }

    if (1 != sscanf(tokens[0].c_str(), "%x", &sender_adr)) {
        LOGDEBUG(logger, " could not parse from address");
        return "";
    }

    if (1 != sscanf(tokens[1].c_str(), "%x", &tmp)) {
        LOGDEBUG(logger, "could not parse to-address");
        return "";
    }

    if (tmp != commadr) {
        LOGDEBUG(logger, "Received string is not for us: Wrong receiver");
        return "";
    }

    if (1 != sscanf(tokens[2].c_str(), "%x", &tmp)) {
        LOGDEBUG(logger, "could not parse telegram length");
        return "";
    }

    if (tmp != s.length()) {
        LOGDEBUG(logger, "wrong telegram length ");
        return "";
    }

    if (tokens[3] != "64") {
        LOGDEBUG(logger, "Simulator only handling port 100");
        return "";
    }

    std::string ret;
    std::string tmps;
    bool found;
    int j = 0;

    for (i = 4; i < tokens.size() - 1; i++) {
        // tokens[i] contains the commands to be answered.
        //LOGDEBUG(logger,"token=" << tokens[i]);
        found = false;
        for (j = 0; scommands[j].token; j++) {
            if (tokens[i] == scommands[j].token) {
                if (scommands[j].value) {
                    found = true;
                    // LOGTRACE(logger, tokens[i] << " found");
                    tmps = convert2sputnikhex(scommands[j].value,
                            scommands[j].scale1);
                    if (!tmps.empty()) {
                        if (!ret.empty()) {
                            ret += ";";
                        }
                        ret += tokens[i] + "=" + tmps;
                    } else {
                        continue;
                    }
                }
                if (scommands[j].value2) {
                    ret += ","
                            + convert2sputnikhex(scommands[j].value2,
                                    scommands[j].scale2);
                }
                break; // token handled. continue with the next
            }
        }
        // token not in the list
        if (!found) LOGINFO(logger, "Token " << tokens[i] << " unknown and not answered");
    }

    // ret contains token answer, but without framing.
    // size of answer = header {12;12;12|64:<ret>|1234}
    //                         1234567890123     456789
    char buf[32];
    unsigned int telsize = ret.length() + 19;
    snprintf(buf, sizeof(buf), "{%02X;%02X;%02X|64:", commadr, sender_adr,
            telsize);
    ret = buf + ret + "|";
    unsigned int checksum = CalcChecksum(ret.c_str(), ret.length());
    snprintf(buf, sizeof(buf), "%04X}", checksum);
    ret += buf;
    return ret;

}

void CInverterSputnikSSeriesSimulator::tokenizer(const char *delimiters,
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
