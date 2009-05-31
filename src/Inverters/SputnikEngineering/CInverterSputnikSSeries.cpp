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

 This programm is distributed in the hope that it will be useful, but
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

#include "configuration/Registry.h"

#include "CInverterSputnikSSeries.h"

#include <libconfig.h++>
#include <iostream>
#include <sstream>
#include <string>
#include "patterns/ICommand.h"
#include "interfaces/CWorkScheduler.h"

#include "Inverters/Capabilites.h"
#include "patterns/CValue.h"


static struct {
		unsigned int typ;
		const char *description;
} model_lookup[] = {
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
		{ 20030, "SolarMax 4200 S"},
		{ 20040, "SolarMax 6000 S"},
		{ 20,	"SolarMax 20 C" },
		{ 25,	"SolarMax 25 C" },
		{ 30,	"SolarMax 30 C" },
		{ 35,	"SolarMax 35 C" },
		{ 50,	"SolarMax 50 C" },
		{ 80,	"SolarMax 80 C" },
		{ 100,	"SolarMax 100 C" },
		{ 300,	"SolarMax 300 C" },
		{ -1, 	"UKNOWN MODEL. PLEASE FILE A BUG WITH THE REPORTED ID, ALONG WITH ALL INFOS YOU HAVE"}
};


using namespace std;


CInverterSputnikSSeries::CInverterSputnikSSeries(const string &name, const string & configurationpath)
: IInverterBase::IInverterBase(name, configurationpath)
{

	// Add the capabilites that this inverter has
	// Note: The "must-have" ones are already instanciated by the base class constructor.
	// Note2: You also can add capabilites as soon you know them (runtime detection)

	string s;
	IValue *v;
	CCapability *c;
	s = CAPA_INVERTER_MANUFACTOR_NAME;
	v = IValue::Factory( CAPA_INVERTER_MANUFACTOR_TYPE );
	((CValue<string>*) v)->Set("Sputnik Engineering");
	c = new CCapability(s , v, this );
	AddCapability(s,c);

	// Add the request to initialize as soon as the system is up.
	ICommand *cmd = new ICommand(CMD_INIT, this, 0 );
	Registry::GetMainScheduler()->ScheduleWork(cmd);

//	libconfig::Setting &set = Registry::Instance().GetSettingsForObject(configurationpath);
//	string setting = "ownadr";
//	if(set.lookupValue(setting,ownadr)) ownadr = 0xFB;

//	 setting = "commadr";
//	if(set.lookupValue(setting,commadr)) commadr = 0x01;

}


CInverterSputnikSSeries::~CInverterSputnikSSeries() {
	// TODO Auto-generated destructor stub
}


bool CInverterSputnikSSeries::CheckConfig()
{
	string setting;
	string str;

	bool ret = true;
	// Check, if we have enough informations to work on.
	libconfig::Setting &set = Registry::Instance().GetSettingsForObject(configurationpath);

	setting = "comms";
	if (! set.exists(setting) || !set.getType() ==  libconfig::Setting::TypeString) {
		cerr << "Setting " << setting << " in " << configurationpath << "."
			<< name << " missing or of wrong type (wanted a string)" << endl;
		ret = false;
	}

	// Check config of the component, if already instanciated.
	if (connection ) {
		ret = connection->CheckConfig();
	}

	setting = "commadr";
	if (! set.exists(setting) || !set.getType() ==  libconfig::Setting::TypeInt) {
		cerr << "Setting " << setting << " in " << configurationpath << "."
			<< name << " missing or of wrong type (wanted a integer)" << endl;
		ret = false;
	}

	return ret;
}

/** Calculate the telegram checksum and return it.
 *
 *  The sputnik protects it telegrams with a checksum.
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
	do	{
		chksum += *str++;
		// cout << chksum << " " << len << endl;
	} while (--len);

	return chksum;
}

void CInverterSputnikSSeries::ExecuteCommand(const ICommand *Command)
{
	static int errcnt = 0;

	string commstring = "";

	ICommand *cmd;

	switch ((Commands)Command->getCmd())
	{

	case CMD_INIT:

		if (!connection->Connect())	{
			cmd = new ICommand(CMD_INIT, this, 0 );
			timespec ts; ts.tv_sec = 15; ts.tv_nsec = 500000;
			Registry::GetMainScheduler()->ScheduleWork(cmd, ts);
			cerr << "offline: scheduling reconnection in " << ts.tv_sec << " seconds" << endl;
			errcnt ++;
			break;
		}

		// Try to identify the inverter.
		pushinverterquery(TYP);
		pushinverterquery(SWV);
		pushinverterquery(BUILDVER);
		pushinverterquery(PAC);
		pushinverterquery(KHR);
		pushinverterquery(DYR);
		pushinverterquery(DMT);
		pushinverterquery(DDY);

		commstring = assemblequerystring();

//		pushinverterquery(EC00);
//		pushinverterquery(EC01);
//		pushinverterquery(EC02);
//		pushinverterquery(EC03);
//		pushinverterquery(EC04);
//		pushinverterquery(EC05);
//		pushinverterquery(EC06);
//		pushinverterquery(EC07);
//		pushinverterquery(EC08);

		cout << " Sent:\t" << commstring << endl;
		connection->Send(commstring);

		// wait for
		cmd = new ICommand(CMD_IDENTFY_WAIT, this, 0 );
		// TODO change that fixed-wait time to the "estimated roundtrip algorithm"
		// which will calculate the expected delay....
		timespec ts; ts.tv_sec = 0; ts.tv_nsec = 300UL * 1000UL;
		Registry::GetMainScheduler()->ScheduleWork(cmd, ts);
		break;

	case (CMD_IDENTFY_WAIT):

		if(!connection->IsConnected()) {
			connection->Disconnect();

			cmd = new ICommand(CMD_INIT, this, 0);
				timespec ts; ts.tv_sec = 3; ts.tv_nsec = 0;
				Registry::GetMainScheduler()->ScheduleWork(cmd, ts);
				cerr << name << " is offline: scheduling reconnection in " << ts.tv_sec << "seconds" << endl;
				errcnt ++;
			break;
		}

		if (! connection->Receive(commstring) || commstring.empty() ) {
			cerr << name << " did not receive answer ... retrying ." << endl;
			cmd = new ICommand(CMD_IDENTFY_WAIT, this, 0 );
			timespec ts; ts.tv_sec = 1; ts.tv_nsec = 300UL * 1000UL;
			Registry::GetMainScheduler()->ScheduleWork(cmd, ts);
			break;
		}

		// fire up the parser.
		parsereceivedstring(commstring);

		//debug
		cerr <<"received:" <<commstring<< endl;
		break;
	}
}

// Add a inverter-query into the queue for later quering...
void CInverterSputnikSSeries::pushinverterquery(enum query q)
{
	cmdqueue.push(q);
}

string CInverterSputnikSSeries::assemblequerystring()
{
	if (cmdqueue.empty()) return "";

	int len = 0;
	int expectedanswerlen = 0;
	int currentport = 0;
	string nextcmd;
	string querystring, tmp;
	bool cont = true;
	char formatbuffer[256];

	do
	{
		switch (cmdqueue.front())
		{

			case TYP: {
				if (currentport && currentport != QUERY) {
					// Changing ports are not supported.
					// Different ports means a different message.
					cont = false; break;
				}
				currentport = QUERY;
				nextcmd = "TYP";
				expectedanswerlen += 9;
				break;
			}

			case SWV: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY;
				nextcmd = "SWV";
				expectedanswerlen += 6;
				break;
			}

			case BUILDVER: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY;
				nextcmd = "BDN";
				expectedanswerlen += 8;
				break;
			}

			case EC00: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC00";
				expectedanswerlen+=27;
				break;
			}

			case EC01: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC01";
				expectedanswerlen+=27;
				break;
			}

			case EC02: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC02";
				expectedanswerlen+=27;
				break;
			}

			case EC03: {

				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC03";
				expectedanswerlen+=27;
				break;
			}

			case EC04: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC04";
				expectedanswerlen+=27;
				break;
			}

			case EC05: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC05";
				expectedanswerlen+=27;
				break;
			}

			case EC06: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC06";
				expectedanswerlen+=27;
				break;
			}

			case EC07:	{
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC07";
				expectedanswerlen+=27;
				break;
			}

			case EC08:	{
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC08";
				expectedanswerlen+=27;
				break;
			}

			case PAC: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "PAC";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case KHR: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "KHR";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case DYR: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "DYR";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case DMT: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "DMT";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case DDY: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "DDY";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case KYR: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "KYR";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case KMT: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "KMT";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case KDY: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "DMT";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case KT0: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "KT0";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case PIN: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "PIN";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case TNP: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "TNP";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case PRL: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "RPL";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case UDC: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "UDC";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case UL1: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "UL1";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case UL2: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "UL2";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case UL3: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "UL3";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case IDC: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "IDC";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case IL1: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "IL1";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case IL2: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "IL2";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case IL3: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "IL3";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case TKK: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "TKK";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case TK2: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "TK2";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case TK3: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "TK3";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case TMI: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "TMI";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}

			case THR: {
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "THR";
				// TODO check answer len
				expectedanswerlen+=10;
				break;
			}
		}

	if( cont) {
	// check if command will fit into max. telegram len
		// note: 255 = max len, 15 = header "{xx;yy;zz|pppp:", 6 = tail "|CHKS}"
		// (plus one byte for the seperator.)
		// note: this is not squeezed to its end, as we will usually not
		// fill up to 255 bytes because lacking commands...
		if (nextcmd.length() + len <= (255-15-6)-1  &&
				// use our assumption to limit the expected receive telegram
				// add a safety margin of 10.
				expectedanswerlen <= (255-15-6-1-10))  {
			// Check if we need to insert a seperator.
			if(len) {
				querystring += ";";
				len++;
			}
			// Add the prepared command and remove it from the queue.
			querystring += nextcmd;
			len += nextcmd.length();
			cmdqueue.pop();
		}
		else
		{
			cont = false;
		}
	}
	}  while (cont && !cmdqueue.empty() );

	unsigned int ownadr, commadr;

	// Query settings needed and default all optional settings.
	libconfig::Setting &set = Registry::Instance().GetSettingsForObject(configurationpath);
	string setting = "ownadr";
	if(! set.lookupValue(setting,ownadr)) ownadr = 0xFB;

    setting = "commadr";
	if(! set.lookupValue(setting,commadr)) commadr = 0x01;


	sprintf(formatbuffer,"%X:",currentport);
	len = strlen(formatbuffer) + querystring.length() + 10 + 6;

	// finally prepare Header "{<from>;<to>;<len>|<port>:"
	sprintf(formatbuffer,"{%02X;%02X;%02X|%X:",
			ownadr, commadr, len , currentport);

	tmp = formatbuffer + querystring + '|';
	querystring = tmp;

	sprintf(formatbuffer, "%04X}",
			CalcChecksum(querystring.c_str(), querystring.length()));

	querystring += formatbuffer;

	// feedback for debugging
	cout << "generated:"<<endl<<querystring<<endl;

	return querystring;
}

bool CInverterSputnikSSeries::parsereceivedstring(const string & s)
{

#if 1
	cerr << "Received:\t" << s << endl;
#endif

	unsigned int i;
	// check for basic constraints...
	if ( s[0] != '{' || s[s.length()-1] != '}')
		return false;

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


#if 1
	// Deubg: Ausgabe aller tokens:
	{
	vector<string>::iterator it;
	for ( i=0,it = tokens.begin(); it != tokens.end() - 1; it++)
	{
		cerr << i++ << ": " << (*it) << "\tlen: " << (*it).length() << endl;
	}
	}
#endif

	unsigned int tmp;
	if ( 1 != sscanf(tokens.back().c_str(), "%x", &tmp)) {
		cerr << " could not parse checksum "<< endl;
		return false;
	}

	if ( tmp != CalcChecksum(s.c_str(), s.length()-6)) {
			cerr << " Checksum error on received telegram" << endl;
			// return false;
		}

	if ( 1 != sscanf(tokens[0].c_str(), "%x", &tmp)) {
			cerr << " could not parse from address "<< endl;
			return false;
	}

	unsigned int ownadr, commadr;

	// Query settings needed and default all optional settings.
	libconfig::Setting &set = Registry::Instance().GetSettingsForObject(configurationpath);
	string setting = "ownadr";
	if(! set.lookupValue(setting,ownadr)) ownadr = 0xFB;

    setting = "commadr";
	if(! set.lookupValue(setting,commadr)) commadr = 0x01;

	if ( tmp != commadr) {
		cerr << "not for us: Wrong Sender " << endl;
		// TODO : Here's a right place to tell the communication interface that
		// this telegram is not for our instance.
		return false;
	}

	if ( 1 != sscanf(tokens[1].c_str(), "%x", &tmp)) {
			cerr << " could not parse to-address "<< endl;
			return false;
	}

	if ( tmp != ownadr) {
		cerr << "not for us: Wrong receiver " << endl;
		return false;
	}

	if ( 1 != sscanf(tokens[2].c_str(), "%x", &tmp)) {
		cerr << " could not parse telegram length "<< endl;
		return false;
	}

	if (tmp != s.length()) {
		cerr << " wrong telegram length "<< endl;
		return false;
	}

	// TODO FIXME: Currently the data port is simply "ignored"
	// parsetoken should get a second argument to get the port infos.

	// okay, done...
	for(i=4; i<tokens.size()-1; i++)
	{
		if (!parsetoken(tokens[i])) {
			cout << "BUG: Parse Error at token "<< tokens[i] << ". Received: "<< s << endl
				 << "If the token is unkown or you subject a bug, please report it giving the  token ans received string"
				 << endl;
		}
	}



#if 0
	HIER GEHTS WEITER
	- Tokenizer geht
	- nun checksumme extrahieren,
	- addressen überprüfen
	- port checken
	- und tokens einzeln an einen Tokenparser verfüttern. der daraus die Daten erzeugt.
#endif
	return false;
}


bool CInverterSputnikSSeries::parsetoken(string token) {

	vector<string> subtokens;
	const char delimiters[] = "=,";
	tokenizer(delimiters, token, subtokens);

	// TODO rewrite this section: Lookup the strings and functions in a table, call
	// by function pointer.

	if (subtokens[0] == "TYP")
	{
		return token_TYP(subtokens);
	}

	if (subtokens[0] == "SWV")
	{
		return token_SWVER(subtokens);
	}

	if (subtokens[0] == "BDN")
	{
		return token_BUILDVER(subtokens);
	}

	if (subtokens[0].substr(0,2) == "EC")
	{
		return token_ECxx(subtokens);
	}

	if (subtokens[0] == "PAC")
	{
		return token_PAC(subtokens);
	}

	if (subtokens[0] == "KHR")
	{
		return token_KHR(subtokens);
	}

	if (subtokens[0] == "DYR")
	{
		return token_DYR(subtokens);
	}

	if (subtokens[0] == "DMT")
	{
		return token_DMT(subtokens);
	}

	if (subtokens[0] == "DDY")
	{
		return token_DDY(subtokens);
	}

	if (subtokens[0] == "KYR")
	{
		return token_KYR(subtokens);
	}

	if (subtokens[0] == "KMT")
	{
		return token_KMT(subtokens);
	}

	if (subtokens[0] == "KDY")
	{
		return token_KDY(subtokens);
	}

	if (subtokens[0] == "KT0")
	{
		return token_KT0(subtokens);
	}

	if (subtokens[0] == "PIN")
	{
		return token_PIN (subtokens);
	}

	if (subtokens[0] == "TNP")
	{
		return token_TNP(subtokens);
	}
	if (subtokens[0] == "PRL")
	{
		return token_PRL(subtokens);
	}
	if (subtokens[0] == "UDC")
	{
		return token_UDC (subtokens);
	}

	if (subtokens[0] == "UL1")
	{
		return token_UL1  (subtokens);
	}

	if (subtokens[0] == "UL2")
	{
		return token_UL2  (subtokens);
	}

	if (subtokens[0] == "UL3")
	{
		return token_UL3 (subtokens);
	}

	if (subtokens[0] == "IDC")
	{
		return token_IDC(subtokens);
	}
	if (subtokens[0] == "IL1")
	{
		return token_IL1(subtokens);
	}

	if (subtokens[0] == "IL2")
	{
		return token_IL2(subtokens);
	}

	if (subtokens[0] == "IL3")
	{
		return token_IL3(subtokens);
	}

	if (subtokens[0] == "TKK")
	{
		return token_TKK(subtokens);
	}

	if (subtokens[0] == "TK2")
	{
		return token_TK2(subtokens);
	}

	if (subtokens[0] == "TK3")
	{
		return token_TK3(subtokens);
	}

	if (subtokens[0] == "TMI")
	{
		return token_TMI(subtokens);
	}

	if (subtokens[0] == "THR")
	{
		return token_THR(subtokens);
	}

	return true;
}



void CInverterSputnikSSeries::tokenizer(const char *delimiters,  const string& s , vector<string> &tokens)
{
	unsigned int i;

	string::size_type lastPos = 0;
	string::size_type pos;

	i=0;
	// Skip tokens at the start of the string
	do	{
		if (s[lastPos] == delimiters[i] )  {
			lastPos++; i=0;
		}
	}while(++i < strlen(delimiters));

	// get the first substring by finding the "second" delimieter
	i= lastPos;
	do {
		unsigned int tmp;
		tmp = s.find_first_of(delimiters[i], lastPos);
		if (tmp < pos ) pos = tmp;
	}while(++i < strlen(delimiters));

	while (s.length() > pos && s.length() > lastPos)
	{
		unsigned int tmp, tmp2;

		// Add it to the vector.
		tokens.push_back(s.substr(lastPos, pos - lastPos));
		lastPos = pos;

		// Skip delimiters.
		i=0;
		do
		{
			if (s[lastPos] == delimiters[i] )  {
				lastPos++; i=0;
			}

		}while(++i < strlen(delimiters));

		// Find next "delimiter"
		i=0; tmp2 = -1;
		do
		{
			tmp = s.find_first_of(delimiters[i], lastPos);
			if (tmp < tmp2 ) tmp2 = tmp;
		}while(++i < strlen(delimiters));
		pos = tmp2;
	}

	// Check if we have an "end-token" (not seperated)
	if (lastPos != s.length()) 	{
		tokens.push_back(s.substr(lastPos, s.length() - lastPos));
	}
}



bool CInverterSputnikSSeries::token_TYP(const vector<string> & tokens)
{

	string strmodel;
	unsigned int i = 0;

	// Check syntax
	if (tokens.size() != 2) return false;
	unsigned int model;
	sscanf(tokens[1].c_str(),"%x", &model);

	do	{
		if(model_lookup[i].typ == model) break;
	} while(model_lookup[++i].typ != (unsigned int )-1);

	if (model_lookup[i].typ == (unsigned int )-1 )
	{
		cerr << "Identified a " << model_lookup[i].description << endl;
		cerr << "Received TYP was " << tokens[0] <<"=" << tokens[1] << endl;
	}

	// lookup if we already know that informations.
	CCapability *cap = GetConcreteCapability( CAPA_INVERTER_MODEL_NAME );

	if (! cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_MODEL_NAME;
		v = IValue::Factory( CAPA_INVERTER_MODEL_TYPE );
		((CValue<string>*) v)->Set(model_lookup[i].description);
		c = new CCapability(s , v, this );
		AddCapability(s,c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if(cap->value->GetType() == CAPA_INVERTER_MODEL_TYPE )	{
		CValue<string> *val = (CValue<string>*) cap->value;
		if (model_lookup[i].description != val->Get()) {
			cerr << "WARNING: Updating inverter type from " << val->Get() << " to " <<
				model_lookup[i].description << endl;
			val->Set(model_lookup[i].description);
			cap->Notify();
		}
	}
	else
	{
		cerr << "BUG: " << CAPA_INVERTER_MODEL_NAME << " not a string ";
	}

	return true;
}


bool CInverterSputnikSSeries::token_SWVER(const vector<string> & tokens)
{
	int tmp;
	string ver;

	// Check syntax
	if (tokens.size() != 2) return false;

	sscanf(tokens[1].c_str(),"%x",&tmp);

	// Been there, seen that.
	if ( swversion == tmp) return true;
	swversion = tmp;

	create_versioncapa();

	return true;
}


bool CInverterSputnikSSeries::token_BUILDVER(const vector<string> & tokens)
{
	int tmp;
	string ver;

	// Check syntax
	if (tokens.size() != 2) return false;

	sscanf(tokens[1].c_str(),"%x",&tmp);

	// Been there, seen that.
	if ( swbuild == tmp) return true;

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
	if (tokens.size() != 2 ) return false;

	unsigned int pac;
	float fpac;
	sscanf(tokens[1].c_str(), "%x", &pac);

	fpac = pac / 2.0;

	// lookup if we already know that informations.
	CCapability *cap = GetConcreteCapability( CAPA_INVERTER_ACPOWER_TOTAL_NAME );

	if (! cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_ACPOWER_TOTAL_NAME;
		v = IValue::Factory( CAPA_INVERTER_ACPOWER_TOTAL_TYPE );
		((CValue<float>*) v)->Set(fpac);
		c = new CCapability(s , v, this );
		AddCapability(s,c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if(cap->value->GetType() == CAPA_INVERTER_ACPOWER_TOTAL_TYPE )	{
		CValue<float> *val = (CValue<float>*) cap->value;

		if ( val -> Get() != fpac ) {
			val->Set(fpac);
			cap->Notify();
		}
	}
	else {
		cerr << "BUG: " << CAPA_INVERTER_ACPOWER_TOTAL_NAME << " not a float ";
	}

	return true;
}

bool CInverterSputnikSSeries::token_KHR(const vector<string> & tokens)
{
	// Operational hours
	if (tokens.size() != 2 ) return false;

	unsigned int pac;
	float fpac;
	sscanf(tokens[1].c_str(), "%x", &pac);

	fpac = pac / 2.0;

	// lookup if we already know that informations.
	CCapability *cap = GetConcreteCapability( CAPA_INVERTER_PON_HOURS_NAME );

	if (! cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_PON_HOURS_NAME;
		v = IValue::Factory( CAPA_INVERTER_PON_HOURS_TYPE  );
		((CValue<float>*) v)->Set(fpac);
		c = new CCapability(s , v, this );
		AddCapability(s,c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if(cap->value->GetType() == CAPA_INVERTER_PON_HOURS_TYPE )	{
		CValue<float> *val = (CValue<float>*) cap->value;
		if ( val -> Get() != fpac ) {
			val->Set(fpac);
			cap->Notify();
		}
	}
	else {
		cerr << "BUG: " << CAPA_INVERTER_PON_HOURS_NAME << " not a float ";
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
	if (tokens.size() != 2 ) return false;

	unsigned int raw;
	float kwh;
	sscanf(tokens[1].c_str(), "%x", &raw);

	kwh = raw;

	// lookup if we already know that informations.
	CCapability *cap = GetConcreteCapability( CAPA_INVERTER_KWH_Y2D_NAME  );

	if (! cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_KWH_Y2D_NAME ;
		v = IValue::Factory( CAPA_INVERTER_KWH_Y2D_TYPE );
		((CValue<float>*) v)->Set(kwh);
		c = new CCapability(s , v, this );
		AddCapability(s,c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if(cap->value->GetType() == CAPA_INVERTER_KWH_Y2D_TYPE )	{
		CValue<float> *val = (CValue<float>*) cap->value;

		if ( val -> Get() != kwh ) {
			val->Set(kwh);
			cap->Notify();
		}
	}
	else {
		cerr << "BUG: " << CAPA_INVERTER_KWH_Y2D_NAME << " not a float ";
	}

	return true;
}

bool CInverterSputnikSSeries::token_KMT(const vector<string> & tokens)
{
	// Unit kwH
	if (tokens.size() != 2 ) return false;

	unsigned int raw;
	float kwh;
	sscanf(tokens[1].c_str(), "%x", &raw);

	kwh = raw;

	// lookup if we already know that informations.
	CCapability *cap = GetConcreteCapability( CAPA_INVERTER_KWH_M2D_NAME  );

	if (! cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_KWH_M2D_NAME ;
		v = IValue::Factory( CAPA_INVERTER_KWH_M2D_TYPE );
		((CValue<float>*) v)->Set(kwh);
		c = new CCapability(s , v, this );
		AddCapability(s,c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if(cap->value->GetType() == CAPA_INVERTER_KWH_M2D_TYPE )	{
		CValue<float> *val = (CValue<float>*) cap->value;

		if ( val -> Get() != kwh ) {
			val->Set(kwh);
			cap->Notify();
		}
	}
	else {
		cerr << "BUG: " << CAPA_INVERTER_KWH_M2D_NAME << " not a float ";
	}

	return true;
}

bool CInverterSputnikSSeries::token_KDY(const vector<string> & tokens)
{
	// Unit 0.1 kwH
	if (tokens.size() != 2 ) return false;

	unsigned int raw;
	float kwh;
	sscanf(tokens[1].c_str(), "%x", &raw);

	kwh = raw / 10.0;

	// lookup if we already know that informations.
	CCapability *cap = GetConcreteCapability( CAPA_INVERTER_KWH_2D_NAME);
	if (! cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_KWH_2D_NAME ;
		v = IValue::Factory( CAPA_INVERTER_KWH_2D_TYPE );
		((CValue<float>*) v)->Set(kwh);
		c = new CCapability(s , v, this );
		AddCapability(s,c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if(cap->value->GetType() == CAPA_INVERTER_KWH_2D_TYPE )	{
		CValue<float> *val = (CValue<float>*) cap->value;

		if ( val -> Get() != kwh ) {
			val->Set(kwh);
			cap->Notify();
		}
	}
	else {
		cerr << "BUG: " << CAPA_INVERTER_KWH_2D_NAME << " not a float ";
	}

	return true;
}

bool CInverterSputnikSSeries::token_KT0(const vector<string> & tokens)
{
	// FIXME TODO Implement me!
	// Unit kwH
	if (tokens.size() != 2 ) return false;

	unsigned int raw;
	float kwh;
	sscanf(tokens[1].c_str(), "%x", &raw);

	kwh = raw;

	// lookup if we already know that informations.
	CCapability *cap = GetConcreteCapability(  CAPA_INVERTER_KWH_TOTAL_NAME );

	if (! cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_KWH_TOTAL_NAME ;
		v = IValue::Factory( CAPA_INVERTER_KWH_TOTAL_TYPE );
		((CValue<float>*) v)->Set(kwh);
		c = new CCapability(s , v, this );
		AddCapability(s,c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if(cap->value->GetType() == CAPA_INVERTER_KWH_TOTAL_TYPE )	{
		CValue<float> *val = (CValue<float>*) cap->value;

		if ( val -> Get() != kwh ) {
			val->Set(kwh);
			cap->Notify();
		}
	}
	else {
		cerr << "BUG: " << CAPA_INVERTER_KWH_TOTAL_NAME << " not a float ";
	}

	return true;
}

bool CInverterSputnikSSeries::token_PIN(const vector<string> & tokens)
{
	// Unit 0.5 Watts
	if (tokens.size() != 2 ) return false;

	unsigned int raw;
	float f;
	sscanf(tokens[1].c_str(), "%x", &raw);

	f = raw * 0.5;
	// lookup if we already know that informations.
	CCapability *cap = GetConcreteCapability( CAPA_INVERTER_INSTALLEDPOWER_NAME );

	if (! cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_INSTALLEDPOWER_NAME ;
		v = IValue::Factory( CAPA_INVERTER_INSTALLEDPOWER_TYPE );
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s , v, this );
		AddCapability(s,c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if(cap->value->GetType() ==  CAPA_INVERTER_INSTALLEDPOWER_TYPE )	{
		CValue<float> *val = (CValue<float>*) cap->value;

		if ( val -> Get() != f ) {
			val->Set(f);
			cap->Notify();
		}
	}
	else {
		cerr << "BUG: " <<  CAPA_INVERTER_INSTALLEDPOWER_NAME << " not a float ";
	}

	return true;
}

bool CInverterSputnikSSeries::token_TNP(const vector<string> & tokens)
{
	// FIXME TODO Implement me!
	// Unit us
	// f = 1 / T , T = x / 1E6
	if (tokens.size() != 2 ) return false;

	unsigned int raw;
	float f;
	sscanf(tokens[1].c_str(), "%x", &raw);

	f = (1.0 / raw) * 1.0E6;

	// lookup if we already know that informations.
	CCapability *cap = GetConcreteCapability( CAPA_INVERTER_NET_FREQUENCY_NAME );

	if (! cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_NET_FREQUENCY_NAME ;
		v = IValue::Factory( CAPA_INVERTER_NET_FREQUENCY_TYPE );
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s , v, this );
		AddCapability(s,c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if(cap->value->GetType() ==  CAPA_INVERTER_NET_FREQUENCY_TYPE )	{
		CValue<float> *val = (CValue<float>*) cap->value;

		if ( val -> Get() != f ) {
			val->Set(f);
			cap->Notify();
		}
	}
	else {
		cerr << "BUG: " <<  CAPA_INVERTER_NET_FREQUENCY_NAME << " not a float ";
	}

	return true;
}

bool CInverterSputnikSSeries::token_PRL(const vector<string> & tokens)
{
	// Unit 1 %
	if (tokens.size() != 2 ) return false;

	unsigned int raw;
	float f;
	sscanf(tokens[1].c_str(), "%x", &raw);

	f = raw;
	// lookup if we already know that informations.
	CCapability *cap = GetConcreteCapability( CAPA_INVERTER_RELPOWER_NAME );

	if (! cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_RELPOWER_NAME ;
		v = IValue::Factory( CAPA_INVERTER_RELPOWER_TYPE );
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s , v, this );
		AddCapability(s,c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if(cap->value->GetType() ==  CAPA_INVERTER_RELPOWER_TYPE )	{
		CValue<float> *val = (CValue<float>*) cap->value;

		if ( val -> Get() != f ) {
			val->Set(f);
			cap->Notify();
		}
	}
	else {
		cerr << "BUG: " <<  CAPA_INVERTER_RELPOWER_NAME << " not a float ";
	}

	return true;

	// FIXME TODO Implement me!

	return true;
}

bool CInverterSputnikSSeries::token_UDC(const vector<string> & tokens)
{
	// Unit 0.1 Volts
	if (tokens.size() != 2 ) return false;

	unsigned int raw;
	float f;
	sscanf(tokens[1].c_str(), "%x", &raw);

	f = raw * 0.1;
	// lookup if we already know that informations.
	CCapability *cap = GetConcreteCapability( CAPA_INVERTER_INPUT_DC_VOLTAGE_NAME );

	if (! cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_INPUT_DC_VOLTAGE_NAME ;
		v = IValue::Factory( CAPA_INVERTER_INPUT_DC_VOLTAGE_TYPE );
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s , v, this );
		AddCapability(s,c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if(cap->value->GetType() ==  CAPA_INVERTER_INPUT_DC_VOLTAGE_TYPE )	{
		CValue<float> *val = (CValue<float>*) cap->value;

		if ( val -> Get() != f ) {
			val->Set(f);
			cap->Notify();
		}
	}
	else {
		cerr << "BUG: " <<  CAPA_INVERTER_INPUT_DC_VOLTAGE_NAME << " not a float ";
	}

	return true;
}

bool CInverterSputnikSSeries::token_UL1(const vector<string> & tokens)
{	// Unit 0.1 Volts
	if (tokens.size() != 2 ) return false;

	unsigned int raw;
	float f;
	sscanf(tokens[1].c_str(), "%x", &raw);

	f = raw * 0.1;
	// lookup if we already know that informations.
	CCapability *cap = GetConcreteCapability( CAPA_INVERTER_GRID_AC_VOLTAGE_NAME );

	if (! cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_GRID_AC_VOLTAGE_NAME ;
		v = IValue::Factory( CAPA_INVERTER_GRID_AC_VOLTAGE_TYPE );
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s , v, this );
		AddCapability(s,c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if(cap->value->GetType() ==  CAPA_INVERTER_GRID_AC_VOLTAGE_TYPE )	{
		CValue<float> *val = (CValue<float>*) cap->value;

		if ( val -> Get() != f ) {
			val->Set(f);
			cap->Notify();
		}
	}
	else {
		cerr << "BUG: " <<  CAPA_INVERTER_GRID_AC_VOLTAGE_NAME << " not a float ";
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
{	// Unit 0.01 Amps
	if (tokens.size() != 2 ) return false;

	unsigned int raw;
	float f;
	sscanf(tokens[1].c_str(), "%x", &raw);

	f = raw * 0.01;
	// lookup if we already know that informations.
	CCapability *cap = GetConcreteCapability( CAPA_INVERTER_INPUT_DC_CURRENT_NAME );

	if (! cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_INPUT_DC_CURRENT_NAME ;
		v = IValue::Factory( CAPA_INVERTER_INPUT_DC_CURRENT_TYPE );
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s , v, this );
		AddCapability(s,c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if(cap->value->GetType() ==  CAPA_INVERTER_INPUT_DC_CURRENT_TYPE )	{
		CValue<float> *val = (CValue<float>*) cap->value;

		if ( val -> Get() != f ) {
			val->Set(f);
			cap->Notify();
		}
	}
	else {
		cerr << "BUG: " <<  CAPA_INVERTER_INPUT_DC_CURRENT_NAME << " not a float ";
	}

	return true;
}

bool CInverterSputnikSSeries::token_IL1(const vector<string> & tokens)
{
	// unit: 0.01 Amps
	if (tokens.size() != 2 ) return false;

	unsigned int raw;
	float f;
	sscanf(tokens[1].c_str(), "%x", &raw);

	f = raw * 0.01;
	// lookup if we already know that informations.
	CCapability *cap = GetConcreteCapability( CAPA_INVERTER_GRID_AC_CURRENT_NAME );

	if (! cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_GRID_AC_CURRENT_NAME ;
		v = IValue::Factory( CAPA_INVERTER_GRID_AC_CURRENT_TYPE );
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s , v, this );
		AddCapability(s,c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if(cap->value->GetType() ==  CAPA_INVERTER_GRID_AC_CURRENT_TYPE )	{
		CValue<float> *val = (CValue<float>*) cap->value;

		if ( val -> Get() != f ) {
			val->Set(f);
			cap->Notify();
		}
	}
	else {
		cerr << "BUG: " <<  CAPA_INVERTER_GRID_AC_CURRENT_NAME << " not a float ";
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
	if (tokens.size() != 2 ) return false;

	unsigned int raw;
	float f;
	sscanf(tokens[1].c_str(), "%x", &raw);

	f = raw;
	// lookup if we already know that informations.
	CCapability *cap = GetConcreteCapability( CAPA_INVERTER_TEMPERATURE_NAME );

	if (! cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_TEMPERATURE_NAME ;
		v = IValue::Factory( CAPA_INVERTER_TEMPERATURE_TYPE );
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s , v, this );
		AddCapability(s,c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if(cap->value->GetType() ==  CAPA_INVERTER_TEMPERATURE_TYPE )	{
		CValue<float> *val = (CValue<float>*) cap->value;

		if ( val -> Get() != f ) {
			val->Set(f);
			cap->Notify();
		}
	}
	else {
		cerr << "BUG: " <<  CAPA_INVERTER_TEMPERATURE_NAME << " not a float ";
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

	unsigned int major, minor;

	// Won't build a version string if the "major" version is unknown.
	if(! swversion) return;

	major = swversion / 10;
	minor = swversion % 10;

	if (swbuild){
		ver = major + "." + minor + '.' + swbuild ;
	}
	else 	{
		ver = major + "." + minor;
	}

	// lookup if we already know that informations.
	CCapability *cap = GetConcreteCapability( CAPA_INVERTER_FIRMWARE_NAME );

	if (! cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_FIRMWARE_NAME;
		v = IValue::Factory( CAPA_CAPAS_UPDATED_TYPE );
		((CValue<string>*) v)->Set(ver);
		c = new CCapability(s , v, this );
		AddCapability(s,c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	else if(cap->value->GetType() == IValue::string_type)	{
		CValue<string> *val = (CValue<string>*) cap->value;
		if (ver != val->Get()) {
			val->Set(ver);
			cap->Notify();
		}
	}
}


