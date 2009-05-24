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

/** \file CInverterSputnikSSeries.cpp
 *
 *  Created on: May 21, 2009
 *      Author: tobi
 */

#include "configuration/Registry.h"

#include "CInverterSputnikSSeries.h"

#include <libconfig.h++>
#include <iostream>
#include "patterns/ICommand.h"
#include "interfaces/CWorkScheduler.h"

using namespace std;

CInverterSputnikSSeries::CInverterSputnikSSeries(const string &name, const string & configurationpath)
: IInverterBase::IInverterBase(name, configurationpath)
{

	// Add the capabilites that this inverter has
	// Note: The "must-have" ones are already instanciated by the base class constructor.
	// Note2: You also can add capabilites as soon you know them (runtime detection)

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
	unsigned int chskum = 0;
	str++;
	do	{
		chskum += *str++;
		cout << '*';
	} while (--len);
	return chskum;
}


void CInverterSputnikSSeries::ExecuteCommand(const ICommand *Command)
{

	string commstring = "";

	ICommand *cmd;

	static int commanddata;

	switch ((Commands)Command->getCmd())
	{

	case CMD_INIT:

		if (!connection->Connect())	{
			cmd = new ICommand(CMD_INIT, this, &commanddata );
			timespec ts; ts.tv_sec = 3; ts.tv_sec = 300 * 1000;
			Registry::GetMainScheduler()->ScheduleWork(cmd, ts);
		}

		// Try to identify the inverter.
		pushinverterquery(TYP);
		pushinverterquery(SWVER);
		pushinverterquery(BUILDVER);

		pushinverterquery(EC00);
		pushinverterquery(EC01);
		pushinverterquery(EC02);
		pushinverterquery(EC03);
		pushinverterquery(EC04);
		pushinverterquery(EC05);
		pushinverterquery(EC06);
		pushinverterquery(EC07);
		pushinverterquery(EC08);

		commstring = assemblequerystring();
		connection->Send(commstring);

		cmd = new ICommand(CMD_IDENTFY_WAIT, this, &commanddata );
		timespec ts; ts.tv_sec = 0; ts.tv_nsec = 300 * 1000;
		Registry::GetMainScheduler()->ScheduleWork(cmd, ts);
		break;


	case (CMD_IDENTFY_WAIT):
		connection->Receive(commstring);

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
	int currentport = 0;
	string nextcmd;
	string querystring, tmp;
	bool cont = true;
	char formatbuffer[256];

	do
	{
		switch (cmdqueue.front())
		{

			case TYP:
				if (currentport && currentport != QUERY) {
					// Changing ports are not supported.
					// Different ports means a different message.
					cont = false; break;
				}
				currentport = QUERY;
				nextcmd = "TYP";
				break;

			case SWVER:
			{
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY;
				nextcmd = "SWV";
				break;
			}

			case BUILDVER:
			{
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY;
				nextcmd = "BDN";
				break;
			}

			case EC00:
			{
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC00";
				break;
			}
			case EC01:
			{
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC01";
				break;
			}
			case EC02:
			{
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC02";
				break;
			}
			case EC03:
			{
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC03";
				break;
			}
			case EC04:
			{
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC04";
				break;
			}
			case EC05:
			{
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC05";
				break;
			}
			case EC06:
			{
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC06";
				break;
			}
			case EC07:
			{
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC07";
				break;
			}
			case EC08:			{
				if (currentport && currentport != QUERY) { cont = false; break; }
				currentport = QUERY; nextcmd = "EC08";
				break;
			}

		}


	cerr << "Q: " << querystring << " N: " << nextcmd << endl;

	if( cont) {
	// check if command will fit into max. telegram len
		// note: 255 = max len, 15 = header "{xx;yy;zz|pppp:", 6 = tail "|CHKS}"
		// (plus one byte for the seperator.)
		// note: this is not squeezed to its end, as we will usually not
		// fill up to 255 bytes because lacking commands...
		if (nextcmd.length() + len <= (255-15-6)-1) {
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

	int ownadr, commadr;

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






