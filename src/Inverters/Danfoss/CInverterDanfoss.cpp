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
    const string &configurationpath)
    :
        IInverterBase(name, configurationpath, "inverter")
{

    std::string s;
    CConfigHelper cfghlp(configurationpath);

    // Preset member variables.
    _invertertype = type;
    _shutdown_requested = false;

    // Load config with sensible defaults
    float interval;
    cfghlp.GetConfig("queryinterval", interval, 5.0f);

    // Query settings needed and default all optional settings.
    cfghlp.GetConfig("response_timeout", _cfg_response_timeout_ms, 3.0F);
    // cfg-file ha unit "seconds", but we need "milliseconds" later.
    _cfg_response_timeout_ms *= 1000.0;

    cfghlp.GetConfig("connection_timeout", _cfg_connection_timeout_ms, 3.0F);
    // cfg-file ha unit "seconds", but we need "milliseconds" later.
    _cfg_connection_timeout_ms *= 1000.0;

    cfghlp.GetConfig("send_timeout", _cfg_send_timeout_ms, 3.0F);
    // cfg-file ha unit "seconds", but we need "milliseconds" later.
    _cfg_send_timeout_ms *= 1000.0;

    cfghlp.GetConfig("reconnect_delay", _cfg_reconnectdelay_s, 15.0F);

    // We'll cache the network adresses here
    cfghlp.GetConfig("inverter_network", _cfg_dest_network_adr);
    cfghlp.GetConfig("inverter_subnet", _cfg_dest_subnet_adr);
    cfghlp.GetConfig("inverter_address", _cfg_dest_adr);

    // Same for the master, but for those sensible defaults exits.
    // Default is  0:0:2
    cfghlp.GetConfig("master_network", _cfg_master_network_adr, 0);
    cfghlp.GetConfig("master_subnet", _cfg_master_subnet_adr, 0);
    cfghlp.GetConfig("master_address", _cfg_master_adr, 2);

    // Complete your initializtion here.
    // For example, add capabilities:
    IValue *v;
    CCapability *c;
    s = CAPA_INVERTER_MANUFACTOR_NAME;
    v = CValueFactory::Factory<CAPA_INVERTER_MANUFACTOR_TYPE>();
    ((CValue<string>*)v)->Set("Danfoss");
    c = new CCapability(s, v, this);
    AddCapability(c);

    s = CAPA_INVERTER_QUERYINTERVAL;
    v = CValueFactory::Factory<CAPA_INVERTER_QUERYINTERVAL_TYPE>();
    ((CValue<float>*)v)->Set(interval);
    c = new CCapability(s, v, this);
    AddCapability(c);

    s = CAPA_INVERTER_CONFIGNAME;
    v = CValueFactory::Factory<CAPA_INVERTER_CONFIGNAME_TYPE>();
    ((CValue<std::string>*)v)->Set(name);
    c = new CCapability(s, v, this);
    AddCapability(c);

    // add a bootstrap event to get called again ;-)
    ICommand *cmd = new ICommand(CMD_INIT, this);
    Registry::GetMainScheduler()->ScheduleWork(cmd);

    LOGDEBUG(logger, "Inverter configuration:");
    LOGDEBUG(logger, "class CInverterDanfoss ");
    cfghlp.GetConfig("comms", s, (string)"unset");
    LOGDEBUG(logger, "Communication: " << s);

    // Register for broadcast events
    Registry::GetMainScheduler()->RegisterBroadcasts(this);
}

CInverterDanfoss::~CInverterDanfoss()
{
}

bool CInverterDanfoss::CheckConfig()
{
#warning needs review -- check if all parameters make sense for Danfoss and if any are missing.
    string setting;
    string str;

    bool fail = false;

    CConfigHelper hlp(configurationpath);

    fail |= (true != hlp.CheckConfig("comms", libconfig::Setting::TypeString));
    // Note: Queryinterval is optional. But CConfigHelper handle also opt.
    // parameters and checks for type.
    fail |= (true != hlp.CheckConfig("queryinterval", libconfig::Setting::TypeFloat, true));
    fail |= (true != hlp.CheckConfig("commadr", libconfig::Setting::TypeInt));

    // Check config of the communication component, if already instanciated.
    if (connection) {
        fail |= (true != connection->CheckConfig());
    }

    // syntax check for communication timeouts
    fail |= (true != hlp.CheckConfig("response_timeout", libconfig::Setting::TypeFloat, true));
    fail |= (true != hlp.CheckConfig("connection_timeout", libconfig::Setting::TypeFloat, true));
    fail |= (true != hlp.CheckConfig("send_timeout", libconfig::Setting::TypeFloat, true));
    fail |= (true != hlp.CheckConfig("reconnect_delay", libconfig::Setting::TypeFloat, true));

    // syntax and range check for all the adresses.
    fail |= (true != hlp.CheckConfig("inverter_network", libconfig::Setting::TypeInt));
    fail |= (true != hlp.CheckConfig("inverter_subnet", libconfig::Setting::TypeInt));
    fail |= (true != hlp.CheckConfig("inverter_address", libconfig::Setting::TypeInt));
    fail |= (true != hlp.CheckConfig("master_network", libconfig::Setting::TypeInt, true));
    fail |= (true != hlp.CheckConfig("master_subnet", libconfig::Setting::TypeInt, true));
    fail |= (true != hlp.CheckConfig("master_address", libconfig::Setting::TypeInt, true));

    // range-checks
    // note: The variables have been initialized in the constructor.
    if (!fail) {
        if (_cfg_dest_network_adr > 14 || _cfg_dest_network_adr < 0) {
            LOGERROR(logger,
                     "inverter_network invalid. Must be between 0 and 14.");
            fail = true;
        }
        if (_cfg_dest_subnet_adr > 14 || _cfg_dest_subnet_adr < 0) {
            LOGERROR(logger,
                     "inverter_subnet invalid. Must be between 0 and 14.");
            fail = true;
        }
        if (_cfg_dest_adr > 254 || _cfg_dest_adr < 0) {
            LOGERROR(logger,
                     "inverter_address invalid. Must be between 0 and 254.");
            fail = true;
        }

        if (_cfg_master_network_adr > 14 || _cfg_master_network_adr < 0) {
            LOGERROR(logger,
                     "master_network invalid. Must be between 0 and 14.");
            fail = true;
        }
        if (_cfg_master_subnet_adr > 14 || _cfg_master_subnet_adr < 0) {
            LOGERROR(logger,
                     "master_subnet invalid. Must be between 0 and 14.");
            fail = true;
        }
        if (_cfg_master_adr > 254 || _cfg_master_adr < 0) {
            LOGERROR(logger,
                     "master_address invalid. Must be between 0 and 254.");
            fail = true;
        }

        // Warnings -- they are non-fatal...
        if ( _cfg_master_network_adr ) {
            LOGWARN(logger, "master_network should be 0.");
        }
        if ( ! _cfg_dest_network_adr ) {
            LOGWARN(logger, "dest_network should not be 0.");
        }

    }

    LOGTRACE(logger, "Check Configuration result: " << !fail);
    return !fail;
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
