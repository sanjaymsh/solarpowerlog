/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2011-2014 Tobias Frost

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

/*
 * CInverterDanfoss.cpp
 *
 *  Created on: 18.05.2014
 *      Author: coldtobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_INV_DANFOSS

#include "Inverters/Danfoss/CInverterDanfoss.h"
#include "configuration/CConfigHelper.h"
#include "Inverters/Capabilites.h"
#include "patterns/IValue.h"
#include "interfaces/CCapability.h"
#include "patterns/CValue.h"
#include "patterns/ICommand.h"

#warning TODO

CInverterDanfoss::CInverterDanfoss(const string &type, const string &name,
		const string &configurationpath) :
		IInverterBase(name, configurationpath, "inverter")
{
	CConfigHelper cfghlp(configurationpath);
	std::string s;

	// Complete your initializtion here.
	// For example, add capabilities:
	IValue *v;
	CCapability *c;
	s = CAPA_INVERTER_MANUFACTOR_NAME;
	v = CValueFactory::Factory<CAPA_INVERTER_MANUFACTOR_TYPE>();
	((CValue<string>*) v)->Set("Danfoss");
	c = new CCapability(s, v, this);
	AddCapability(c);

	// add a bootstrap event to get called again ;-)
	ICommand *cmd = new ICommand(CMD_INIT, this);
	Registry::GetMainScheduler()->ScheduleWork(cmd);

	LOGDEBUG(logger,"Inverter configuration:");
	LOGDEBUG(logger,"class CInverterDanfoss ");
	cfghlp.GetConfig("comms", s, (string) "unset");
	LOGDEBUG(logger,"Communication: " << s);
}

CInverterDanfoss::~CInverterDanfoss()
{
}


void CInverterDanfoss::ExecuteCommand(const ICommand *Command)
{
	// ICommandTarget hook -- will be called with any new command which is due..

	LOGINFO(this->logger,
			"CInverterDanfoss " << this->name << " command received! GetCmd:" <<
			Command->getCmd());
	LOGINFO(this->logger, "Data:");
#ifdef ICMD_WITH_DUMP_MEMBER
	Command->DumpData(this->logger);
#endif

	// Probably you want to have a big switch-case here, covering all your CMD_xxx
}

#endif /* HAVE_INV_DUMMY */
