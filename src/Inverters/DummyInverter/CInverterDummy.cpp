/*
 * CInverterDummy.cpp
 *
 *  Created on: 17.07.2011
 *      Author: coldtobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_INV_DUMMY

#include "Inverters/DummyInverter/CInverterDummy.h"
#include "configuration/CConfigHelper.h"
#include "Inverters/Capabilites.h"
#include "patterns/IValue.h"
#include "interfaces/CCapability.h"
#include "patterns/CValue.h"
#include "patterns/ICommand.h"

CInverterDummy::CInverterDummy(const string &name,
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
	v = IValue::Factory(CAPA_INVERTER_MANUFACTOR_TYPE);
	((CValue<string>*) v)->Set("Dummy Inverter");
	c = new CCapability(s, v, this);
	AddCapability(s, c);

	// add a bootstrap event to get called again ;-)
	ICommand *cmd = new ICommand(CMD_INIT, this);
	Registry::GetMainScheduler()->ScheduleWork(cmd);

	LOGDEBUG(logger,"Inverter configuration:");
	LOGDEBUG(logger,"class CInverterDummy ");
	cfghlp.GetConfig("comms", s, (string) "unset");
	LOGDEBUG(logger,"Communication: " << s);
}

CInverterDummy::~CInverterDummy()
{
	// TODO Auto-generated destructor stub
}



void CInverterDummy::ExecuteCommand(const ICommand *Command)
{
	// ICommandTarget hook -- will be called with any new command which is due..

	LOGINFO(this->logger,
			"CInverterDummy " << this->name << " command received! GetCmd:" <<
			Command->getCmd());
	LOGINFO(this->logger, "Data:");
	Command->DumpData(this->logger);

	// Probably you want to have a big switch-case here, covering all your CMD_xxx


}



#endif /* HAVE_INV_DUMMY */
