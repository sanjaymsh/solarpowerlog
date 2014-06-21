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

#define DANFOSS_CRC_GOOD (0xf0b8U)

#include "Inverters/Danfoss/CInverterDanfoss.h"
#include "Inverters/Danfoss/DanfossCommand/CDanfossCommand.h"
#include "configuration/CConfigHelper.h"
#include "Inverters/Capabilites.h"
#include "patterns/IValue.h"
#include "interfaces/CCapability.h"
#include "patterns/CValue.h"
#include "patterns/ICommand.h"


// We borrow those...
#include "Inverters/SputnikEngineering/SputnikCommand/BackoffStrategies/CSputnikCmdBOOnce.h"
#include "Inverters/SputnikEngineering/SputnikCommand/BackoffStrategies/CSputnikCmdBOTimed.h"
#include "Inverters/SputnikEngineering/SputnikCommand/BackoffStrategies/CSputnikCmdBOIfSupported.h"

#include <boost/crc.hpp>
#include <boost/cstdint.hpp>

#warning TODO

using namespace DanfossCommand;

std::string hexdump(const std::string &s) {

    std::string st;
    char buf[32];
    for (unsigned int i = 0; i < s.size(); i++) {
        sprintf(buf, "%02x", (unsigned char)s[i]);
        st = st + buf;
        if (i && i % 32 == 0)
        st = st + "\n";
        else
        st = st + ' ';
    }
    return st;
}

void CInverterDanfoss::_localdebug(void) {
    // local debug helper...
    unsigned char testmsg2[] = { 0x7E, 0xFF, 0x03, 0x00, 0x02, 0x7D, 0x5D,
                                 0x7D, 0x5E, 0x00, 0x15, 0x99, 0xC9, 0x7E };

    std::string test, test2;
    test.append( (char*)testmsg2, sizeof(testmsg2));

    LOGDEBUG(logger, "Input string:" << endl << hexdump(test));


    // removing first and last byte (0x7e's)
    test.erase(test.length()-1, 1);
    test.erase(0, 1);

    LOGDEBUG(logger, "Input string after removing frame:" << endl << hexdump(test));

    test = this->hdlc_debytestuff(test);

    LOGDEBUG(logger, "Input string after debytestuff:" << endl << hexdump(test));

    LOGDEBUG(logger, "Checksum is calculated as: 0x" << hex << this->hdlc_calcchecksum(test));

    LOGDEBUG(logger,"Forward calc");
    // Trying to calculate the CRC
    test.erase(test.length()-2, 2);
    LOGDEBUG(logger, "Input string for crccalc:" << endl << hexdump(test));

    unsigned int checksum = this->hdlc_calcchecksum(test);
    checksum ^= 0xffffU;
    LOGDEBUG(logger, "Checksum^0xffff is calculated as: 0x" << hex << (checksum));

    test.push_back(checksum & 0xff);
    test.push_back((checksum >>8 ) & 0xff );

    test = hdlc_bytestuff(test);

    LOGDEBUG(logger,"Output would be:" << endl << hexdump(test));
}

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
    _notansweredcommand = NULL;
    _softtimeout= false;

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
    cfghlp.GetConfig("inverter_network", _cfg_inv_network_adr);
    cfghlp.GetConfig("inverter_subnet", _cfg_inv_subnet_adr);
    cfghlp.GetConfig("inverter_address", _cfg_inv_adr);

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


    // Register all CDanfossCommmands we'd like to handle.
    // Note that there are differences between some models...
    if (type == "UniLynx") {
        commands.push_back(
            new CDanfossCommand<CAPA_INVERTER_KWH_TOTAL_TYPE>
            (logger, 0x01, 0x02, 0x04, DanfossCommand::type_u32, this,
            CAPA_INVERTER_KWH_TOTAL_NAME));
        commands.push_back(
            new CDanfossCommand<CAPA_INVERTER_ACPOWER_TOTAL_TYPE>
            (logger, 0x02, 0x01, 0x0D, DanfossCommand::type_u32, this,
            CAPA_INVERTER_ACPOWER_TOTAL));
        commands.push_back(
            new CDanfossCommand<CAPA_INVERTER_KWH_2D_TYPE>
            (logger, 0x01, 0x04, 0x04, DanfossCommand::type_u32, this,
            CAPA_INVERTER_KWH_2D));
    }

    if (type == "TripleLynx") {
        commands.push_back(
           new CDanfossCommand<CAPA_INVERTER_ACPOWER_TOTAL_TYPE>
            (logger, 0x02, 0x46, 0x08 ,DanfossCommand::type_u32, this,
                 CAPA_INVERTER_ACPOWER_TOTAL));
        commands.push_back(
            new CDanfossCommand<CAPA_INVERTER_KWH_2D_TYPE>
             (logger, 0x02, 0xA7, 0x08, DanfossCommand::type_u32, this,
              CAPA_INVERTER_KWH_2D));
    }

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
        if (_cfg_inv_network_adr > 14 || _cfg_inv_network_adr < 0) {
            LOGERROR(logger,
                     "inverter_network invalid. Must be between 0 and 14.");
            fail = true;
        }
        if (_cfg_inv_subnet_adr > 14 || _cfg_inv_subnet_adr < 0) {
            LOGERROR(logger,
                     "inverter_subnet invalid. Must be between 0 and 14.");
            fail = true;
        }
        if (_cfg_inv_adr > 254 || _cfg_inv_adr < 0) {
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
        if (_cfg_master_network_adr) {
            LOGWARN(logger, "master_network should be 0.");
        }
        if (!_cfg_inv_network_adr) {
            LOGWARN(logger, "inverter_network should not be 0.");
        }

        // Pre-Calculate the addresses we neeed.
        _precalc_masteradr = (_cfg_master_network_adr << 12)
            +  (_cfg_master_subnet_adr << 8) + _cfg_master_adr;

        _precalc_slaveadr = (_cfg_inv_network_adr << 12)
             +  (_cfg_inv_subnet_adr << 8) + _cfg_inv_adr;

    }

    LOGTRACE(logger, "Check Configuration result: " << !fail);
    return !fail;
}

void CInverterDanfoss::ExecuteCommand(const ICommand *Command)
    {

    ICommand *cmd;
    timespec ts;

    LOGTRACE(this->logger,
        "CInverterDanfoss " << this->name << " command received! GetCmd:" <<
        Command->getCmd());

    // Probably you want to have a big switch-case here, covering all your CMD_xxx
    switch ((Commands) Command->getCmd()) {
        case CMD_DISCONNECTED:
        {
            // DISCONNECTED: Error detected, the link to the com partner is down.
            // Action: Schedule connection retry in xxx seconds
            // Next-State: INIT (Try to connect)
            LOGDEBUG(logger, "new state: CMD_DISCONNECTED");

            // Tell everyone that all data is now invalid.
            CCapability *c = GetConcreteCapability(CAPA_INVERTER_DATASTATE);
            CValue<bool> *v = (CValue<bool> *) c->getValue();
            v->Set(false);
            c->Notify();

            // reset the backoff algorithms for the commands.
            pendingcommands.clear();
            _notansweredcommand = NULL;
            vector<ISputnikCommand *>::iterator it;
            for (it=this->commands.begin(); it!=commands.end(); it++) {
                (*it)->InverterDisconnected();
            }

            cmd = new ICommand(CMD_DISCONNECTED_WAIT, this);
            if (connection->IsConnected()) {
                connection->Disconnect(cmd);
            } else {
                Registry::GetMainScheduler()->ScheduleWork(cmd);
            }

            _softtimeout = false;
            break;
        }

        case CMD_DISCONNECTED_WAIT:
        {
            LOGDEBUG(logger, "new state: CMD_DISCONNECTED_WAIT");
            cmd = new ICommand(CMD_INIT, this);
            timespec ts;
            float fraction, intpart;
            fraction = modf(_cfg_reconnectdelay_s, &intpart);
            ts.tv_sec = (long) intpart;
            ts.tv_nsec = (long) (fraction*1E9);
            Registry::GetMainScheduler()->ScheduleWork(cmd, ts);
            break;
        }

        case CMD_INIT:
        {
            LOGDEBUG(logger, "new state: CMD_INIT");
            // initiate new connection only if no shutdown was requested.
            if (_shutdown_requested) break;

            // INIT: Try to connect to the comm partner
            // Action Connection Attempt
            // Next-State: Wait4Connection

            cmd = new ICommand(CMD_WAIT4CONNECTION, this);
            cmd->addData(ICONN_TOKEN_TIMEOUT,((long)_cfg_connection_timeout_ms));
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
            //      success IDENTIFY_COMM
            //      error   DISCONNECTED
            try {
                err = boost::any_cast<int>(Command->findData(ICMD_ERRNO));
            } catch (...) {
                LOGDEBUG(logger,"CMD_WAIT4CONNECTION: unexpected exception");
                err = -1;
            }

            if (err < 0) {
                try {
                    LOGERROR(logger, "Error while connecting: (" << -err << ") " <<
                        boost::any_cast<string>(Command->findData(ICMD_ERRNO_STR)));
                } catch (...) {
                    LOGERROR(logger, "Unknown error while connecting.");
                }

                cmd = new ICommand(CMD_DISCONNECTED, this);
                Registry::GetMainScheduler()->ScheduleWork(cmd);
            } else {
                cmd = new ICommand(CMD_QUERY_POLL, this);
                Registry::GetMainScheduler()->ScheduleWork(cmd);
            }
        }
        break;

        case CMD_QUERY_POLL:
        {
            LOGDEBUG(logger, "new state: CMD_QUERY_POLL ");

            // Collect all queries to be issued.
            std::vector<ISputnikCommand*>::iterator it;
            for (it=commands.begin(); it!= commands.end(); it++) {
                if ((*it)->ConsiderCommand()) {
#ifdef DEBUG_BACKOFFSTRATEGIES
                    LOGTRACE(logger,"Considering Command " << (*it)->command );
#endif
                    pendingcommands.push_back(*it);
                }
#ifdef DEBUG_BACKOFFSTRATEGIES
                else {
                    LOGTRACE(logger," Command " << (*it)->command << " not to be considered.");
                }
#endif
            }

        }
        // fall through intended.

        case CMD_SEND_QUERIES:
        {
            LOGDEBUG(logger, "new state: CMD_SEND_QUERIES ");
            cmd = new ICommand(CMD_WAIT_SENT, this);
            // Start an atomic communication block (to hint any shared comms)
            cmd->addData(ICONN_ATOMIC_COMMS, ICONN_ATOMIC_COMMS_REQUEST);
            cmd->addData(ICONN_TOKEN_SEND_STRING, assemblequerystring());
            cmd->addData(ICONN_TOKEN_TIMEOUT,((long)_cfg_send_timeout_ms));
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
                err = -EINVAL;
            }

            if (err < 0) {
                try {
                    LOGERROR( logger,
                        "Error while sending: (" << -err << ") " << boost::any_cast<string>(Command->findData(ICMD_ERRNO_STR)));
                } catch (...) {
                    LOGERROR(logger, "Error while sending. (" << -err << ")");
                }

                // Hint the shared comms to stop the atomic session
                // and then disconnect.
                cmd = new ICommand(CMD_DISCONNECTED, this);
                cmd->addData(ICONN_ATOMIC_COMMS, ICONN_ATOMIC_COMMS_CEASE);
                connection->Noop(cmd);
                break;
            }

            cmd = new ICommand(CMD_EVALUATE_RECEIVE, this);
            cmd->addData(ICONN_TOKEN_TIMEOUT, (long)_cfg_response_timeout_ms);
            // finish this atomic block (shared comms hinting)
            cmd->addData(ICONN_ATOMIC_COMMS, ICONN_ATOMIC_COMMS_CEASE);
            connection->Receive(cmd);
        }
        break;

        case CMD_EVALUATE_RECEIVE:
        {
#warning: NOT HANDLED: Danfoss inverters *could* not answer an unsupported command, which would here lead to an disconnect and then a reconnect, \
  leading to an loop.... So probably Danfoss needs to treat timeouts not necesserialy as errors, but only if they get massive...


            LOGDEBUG(logger, "new state: CMD_EVALUATE_RECEIVE");

            int err;
            std::string s;
            try {
                err = boost::any_cast<int>(Command->findData(ICMD_ERRNO));
            } catch (...) {
                LOGDEBUG(logger, "BUG: Unexpected exception.");
                err = -EINVAL;
            }

            if (err == -ETIMEDOUT) {
                // Timeout occured -- can be that the inverter simply did not answer

                LOGDEBUG(logger, "Receive timeout");
                assert(_notansweredcommand);
                _notansweredcommand->CommandNotAnswered();
                _notansweredcommand = NULL;

                if (!pendingcommands.empty()) {
                    LOGTRACE(logger, "Querying remaining commands");
                    cmd = new ICommand(CMD_SEND_QUERIES, this);
                    Registry::GetMainScheduler()->ScheduleWork(cmd);
                    break;
                } else {
                    // _softtimeout will be reset to false if *any* telegram
                    // has been answered. If its true here, we've got a permanent
                    // problem.
                    if (_softtimeout) {
                        LOGTRACE(logger, "Soft-timeout: Giving up");
                        _softtimeout = false;
                        cmd = new ICommand(CMD_DISCONNECTED, this);
                        Registry::GetMainScheduler()->ScheduleWork(cmd);
                        break;
                    }
                    // no more pending commands -- repeat the cycle.
                    // but remember that we need at least one good command the
                    // next one, so if it stays true, we'll disconnect.
                    _softtimeout = true;
                    LOGTRACE(logger, "Soft-timeout: Give a new cycle a try");
                    // repeat cycle after delay
                    CCapability *c = GetConcreteCapability(CAPA_INVERTER_QUERYINTERVAL);
                    CValue<float> *v = (CValue<float> *) c->getValue();
                    ts.tv_sec = v->Get();
                    ts.tv_nsec = ((v->Get() - ts.tv_sec) * 1e9);
                    cmd = new ICommand(CMD_QUERY_POLL, this);
                    Registry::GetMainScheduler()->ScheduleWork(cmd, ts);
                    break;
                }

            } else  if (err < 0) {
                // we do not differentiate the error here, an error is an error....
                // try to log the error message, if any.
                try {
                    s = boost::any_cast<std::string>(Command->findData(
                            ICMD_ERRNO_STR));
                    LOGERROR(logger, "Receive Error: (" <<-err <<") "<< s);
                } catch (...) {
                    LOGERROR(logger, "Receive Error: " << strerror(-err));
                }
            }

            try {
                s = boost::any_cast<std::string>(Command->findData(
                        ICONN_TOKEN_RECEIVE_STRING));
            } catch (...) {
                LOGERROR(logger, "Retrieving string: Unexpected Exception");
                err = -EINVAL;
            }

            if (err < 0) {
                // Schedule new connection later.
                cmd = new ICommand(CMD_DISCONNECTED,this);
                Registry::GetMainScheduler()->ScheduleWork(cmd);
                break;
            }

            LOGTRACE(logger, "Received: len=" << s.size() << endl
                << hexdump(s));

            int parseresult = parsereceivedstring(s);

            // TODO parsereceiedstring could handle more than one telegram per received string,
            // but the statemachine currently doesn't... (as error detection becomes non-trivial:
            // What to do if first telegramm OK, second a error or incomplete
            // should we then disconnect or continue... Needs a quiet moment to think about it :)
            // Also the returncode 0 means "not for us" could be handled somehow else than ignoring
            // however this should not happen due to the locking mechanism.

            // parseresult =>
            //      -1 on error,
            //      0 if the string indicated that it is not for us
            //      1 on success.

            if (0 == parseresult) {
                LOGERROR(
                    logger,
                    "Command not for us? (This is currentley handeld same as an error)");
            }

            if (1 != parseresult) {
                // Reconnect on parse errors.
                LOGERROR(logger, "Parse error on received string.");
                cmd = new ICommand(CMD_DISCONNECTED, this);
                Registry::GetMainScheduler()->ScheduleWork(cmd);
                break;
            }

            // the issued command should have been answered,
            // if notansweredcommand is non-null, it has not.
            // So notify the backoff algorithm(s).
            if (_notansweredcommand) {
                _notansweredcommand->CommandNotAnswered();
            } else if(_softtimeout){
                LOGTRACE(logger, "Soft-timeout: Resetted due to received command");
                _softtimeout = false;
            }
            _notansweredcommand = NULL;

            // if there are still pending commands, issue them first before
            // filling the queue again.
            if (!pendingcommands.empty()) {
                LOGTRACE(logger, "Querying remaining commands");
                cmd = new ICommand(CMD_SEND_QUERIES, this);
                Registry::GetMainScheduler()->ScheduleWork(cmd);
                break;
            }

            CCapability *c = GetConcreteCapability(CAPA_INVERTER_DATASTATE);
            CValue<bool> *vb = (CValue<bool> *) c->getValue();
            vb->Set(true);
            c->Notify();

            c = GetConcreteCapability(CAPA_INVERTER_QUERYINTERVAL);
            CValue<float> *v = (CValue<float> *) c->getValue();
            ts.tv_sec = v->Get();
            ts.tv_nsec = ((v->Get() - ts.tv_sec) * 1e9);
            cmd = new ICommand(CMD_QUERY_POLL, this);
            Registry::GetMainScheduler()->ScheduleWork(cmd, ts);

        }
        break;

        // Broadcast events
        case CMD_BRC_SHUTDOWN:
        LOGDEBUG(logger, "new state: CMD_BRC_SHUTDOWN");
        // stop all pending I/Os, as we will exit soon.
        connection->AbortAll();
        _shutdown_requested = true;
        break;

        default:
        if (Command->getCmd() <= BasicCommands::CMD_BROADCAST_MAX) {
            // broadcast event
            LOGDEBUG(logger, "Unhandled broadcast event received " << Command->getCmd());
            break;
        }
        LOGERROR(logger, "Unknown CMD received: "<< Command->getCmd());
        break;
    }

}

/** Check received message for correctness and pass data to CDanfossCommand
 * instance
 *
 * 1) Check if complete telegramm
 * 2) De-bytestuff
 * 1) Check for correctness / completeness
 *     -> CRC
 *     -> Frame
 *     -> Adressing
 * 2) Feed to CDanfossCommand
 *
 *
 *
 * */
int CInverterDanfoss::parsereceivedstring(std::string &rcvd) {

    std::string localrcvd;
    uint16_t tmp;

    // Check first of we've got a complete telegram (that's two 0x7E)
    size_t pos = rcvd.find_first_of(0x7E);
    if (pos != 0 && (std::string::npos != pos)) {
        // Discard all chars in front of the 0x7e
        rcvd = rcvd.substr(pos);
    }

    // Error if there is no 0x7e in the string.
    if ( pos == std::string::npos ) {
        return -1;
    }

    // Look for a/the second 0x7e
    pos = rcvd.find(0x7E, 1);
    if (! pos) return -1; // no second 0x7e -> discard telegram

    if (pos < 12) {
         return -1; // minimum length is 12 bytes (6 for frame, 6 for the message header).
    }

    // Extract telegramm.
    rcvd = rcvd.substr(1,pos-1);
    LOGTRACE(logger, "Telegramm without frame:" << endl << hexdump(rcvd));

    // Check Frame
    if ((unsigned char)rcvd[0] != 0xFF || rcvd[1] != 0x03) {
        LOGDEBUG(logger, "Frame error.");
        return -1;
    }

    rcvd = hdlc_debytestuff(rcvd);
    LOGTRACE(logger, "Telegramm destuffed:" << endl << hexdump(rcvd));

    // now a second size check... After debytestuffing and removing the 0x7e
    // (which could shrink the telgramm) we still need a minimum of 10 bytes
   if (rcvd.length() <= 10 ) {
       LOGDEBUG(logger, "minimum size after destuffing violated");
       return -1;
   }

    if ( DANFOSS_CRC_GOOD != hdlc_calcchecksum(rcvd) ) {
        LOGDEBUG(logger, "Checksum error: 0x"
                 << hex << hdlc_calcchecksum(rcvd));
        return -1;
    }

    // Basic checks done...
    // Lets examine the telegramm closer.
    // remove the frame (2 more bytes -- so we start at the 3rd
    // and remove the CRC (this are the two last bytes)
    // so the new string will be 4 bytes shorter than before
    rcvd = rcvd.substr(2,rcvd.length()-4);

    LOGTRACE(logger, "Message extracted:" << endl << hexdump(rcvd));
    /* now we have in rcvd (size in brackets:
    *
    * |=======================================================|
    * ||                    Header                |   Data   ||
    * ||  Src(2)  | Dest(2) | Size(1)  | Type(1)  |  0-255By ||
    * |=======================================================|
    *
    */

    // lets check if the size matched telegramm size
    tmp = 6 + rcvd[DANFOSS_POS_HDR_SIZE];
    if ( tmp != rcvd.length()) {
        LOGDEBUG(logger, "Message block size mismatch. Received " << rcvd.length() <<
                 ", telegram indicates:" << tmp);
        return -1;
    }

    // now check for application errors.
    // its encoded in the type field,

    tmp = rcvd[DANFOSS_POS_HDR_TYPE];
    if (tmp & (1 << 5)) {
        // Application error bit set
        if (rcvd[DANFOSS_POS_HDR_SIZE] != 1) {
            LOGDEBUG( logger,
                      "Telegramm indicated application error, but error code not 1 byte.");
            return -1;
        }
        // One data bytes tells the error code...
        tmp = rcvd[DANFOSS_POS_DAT_DOCTYPE];
        switch (tmp) {
            case 0x10:
                LOGDEBUG(logger, "Application error bit set: MessageNotSupported");
                break;
            case 0x11:
                LOGDEBUG(logger, "Application error bit set: RequestNotCarriedOut");
                break;
            case 0x12:
                LOGDEBUG(logger, "Application error bit set: IllegalNumberOfDataBytes");
                break;
            case 0xA0:
                LOGDEBUG(logger, "Application error bit set: MissingCAN Response");
                // According docs this can happen if an internal inverter
                // module is not powered up (or if we addressed the wrong moduleid)
                // So the first cause is kind of "soft-error" that might go away over time,
                // so we ignore it here and pretend to be successful.
                // TODO Check when tested on real hardware if this impacts BackOffStrategies...
                return 1;
                break;
            default:
                LOGDEBUG(logger, "Application error bit set: unknown error code " << tmp);
        }
        return -1;
    } else if (tmp & (1 << 6)) {
        // Transmission error bit set
        if (rcvd[DANFOSS_POS_HDR_SIZE] != 1) {
            LOGDEBUG(logger,
                 "Telegramm indicated application error, but error code not 1 byte.");
            return -1;
        }
        // One data byte tells the error code...
        tmp = rcvd[DANFOSS_POS_DAT_DOCTYPE];
        switch (tmp) {
            case 0x01:
                LOGDEBUG(logger, "Transmission error bit set: CRCError");
            break;
            case 0x02:
                LOGDEBUG(logger, "Transmission error bit set: FramingError");
            break;
            case 0x03:
                LOGDEBUG(logger, "Transmission error bit set: BufferOverflow");
            break;
            case 0x04:
                LOGDEBUG(logger, "Transmission error bit set: ByteTimeout");
            break;
            default:
                LOGDEBUG(
                    logger,
                    "Transmission error bit set: unknown error code " << tmp);
        }
        return -1;
    }

    // Check message correctness
    // Adresses:
    //  Source
    tmp = rcvd[DANFOSS_POS_HDR_SOURCE + 1]
                   | (((unsigned char)rcvd[DANFOSS_POS_HDR_SOURCE]) << 8U);

    if (tmp != _precalc_slaveadr) {
        LOGTRACE(logger, "Not from this inverter. 0x"
                 << hex  << tmp << " != 0x"
                 << hex << _precalc_slaveadr
                 << " (<=expected)");
        return 0;
    }

    tmp = rcvd[DANFOSS_POS_HDR_DEST + 1]
          | (((unsigned char)rcvd[DANFOSS_POS_HDR_DEST]) << 8U);

    //  Dest
    if (tmp != _precalc_masteradr) {
        LOGTRACE(logger, "Not for this inverter instance. 0x"
                 << hex << tmp << " != 0x"
                 << hex << _precalc_masteradr
                 << " (<=expected)");
        return 0;
    }

    // Check for application error

    // Validation of telegram done and succeeded.
    // For Danfoss-inverters, there is only one query at a time,
    // so our command must be in _notansweredcommand or there is a problem.
    assert(_notansweredcommand);
    if (!_notansweredcommand->IsHandled(rcvd)) {
        LOGDEBUG(logger, "Received response but not for this commmand. Weird.");
        return -1;
    }

    bool result = _notansweredcommand->handle_token(rcvd);
     if (!result)  {
         LOGTRACE(logger,"failed parsing " + _notansweredcommand->GetCapaName());
         return -1;
     }
     else {
         _notansweredcommand = NULL;
         return 1;
     }
}

/** Generate a telegramm to be sent out.
 *
 * 1) Get (if any) commmand to issue
 * 2) Remove from pending list and add to tracking for nonanswered commands
 * 3) Assemble telegramm (and use CDanfossCommand() to get the message block
 * 4) Add checksum, bytestuff and add frame
 * 5) Ready
 *
 *\returns string to be sent.
 */
std::string CInverterDanfoss::assemblequerystring(void) {

    if (pendingcommands.empty() ) return "";

    std::string t,t2;

    ISputnikCommand *cmd = pendingcommands.front();
    pendingcommands.pop_front();
    _notansweredcommand = cmd;

    t += (char)0xff;
    t += (char)0x03;
    t2 = cmd->GetCommand();
    t2[DANFOSS_POS_HDR_SOURCE] = (_precalc_masteradr >> 8) & 0xFF;
    t2[DANFOSS_POS_HDR_SOURCE + 1] = _precalc_masteradr & 0xFF;

    t2[DANFOSS_POS_HDR_DEST] = (_precalc_slaveadr >> 8) & 0xFF;
    t2[DANFOSS_POS_HDR_DEST + 1] = _precalc_slaveadr & 0xFF;

    t += t2;
    uint16_t crc = hdlc_calcchecksum(t);
    crc ^= 0xffff;

    LOGTRACE(logger, "Checksum 0x" << hex << crc);

    t += (char)(crc & 0xff);
    t += (char)(crc >> 8);

    LOGTRACE(logger, "Checksum reverse check (expected 0xf0b8) 0x" << hex << hdlc_calcchecksum(t));

    t= (char)0x7e +  hdlc_bytestuff(t);
    t += (char)0x7E;

    LOGTRACE(logger, "Telegramm stuffed & ready:" << endl << hexdump(t));

    return t;
}

/** The protocol uses byte-stuffing to avoid appearing magics in the telegram.
 *
 * 0x7D 0x5E -> 0x7E
 * 0x7D 0x5D -> 0x7D
 *
 * \param input input string
 * \returns de-bytestuffed input string.
 *
 * \note errors in the stuffing (e.g 0x7D is not followed
 * by 0x5E or 0x5D are ignored and the bytes dropped.
 * (A Warning will be written to the logger, though)
 */
std::string CInverterDanfoss::hdlc_debytestuff(const std::string& input)
{
    bool was_escaped = false;
    std::string output;
    std::string::const_iterator it = input.begin();
    for (; it != input.end(); it++) {
        if (was_escaped) {
            if (*it == 0x5E) {
                output.push_back(0x7E);
            }
            else if (*it == 0x5D) {
                output.push_back(0x7D);
            }
            else {
                LOGWARN(logger,
                        "Ignoring De-stuffing error in received telegram.");
            }
            was_escaped = false;
        } else if (*it == 0x7D) {
            was_escaped = true;
        } else {
            output.push_back(*it);
        }
    }
    if (was_escaped) {
        LOGWARN(logger, "Ignoring De-stuffing error at end of telegram.");
    }
    return output;
}

/** The protocol uses byte-stuffing to avoid appearing magics in the telegram.
 *
 * The magics are 0x7E and 0x7D, which are escaped with 0x7D 0x5(E|D)
 *
 * \input input string
 * \return bytestuffed string
 *
 * \note the 0x7F marking the start and end should not be fed into this
 * function as those are not to be stuffed.
*/
std::string CInverterDanfoss::hdlc_bytestuff(const std::string& input)
{
    std::string output;
    std::string::const_iterator it = input.begin();
    for (; it != input.end(); it++) {
        if (*it == 0x7D || *it == 0x7E) {
            output.push_back(0x7D);
            output.push_back(*it & 0x5F); // Masks out the single bit...
        } else
        {
            output.push_back(*it);
        }
    }
    return output;
}

/** Calculate the checksum of the protocol.
 *
 * Incoming: Calculate without the frame (the first byte) including to the checksunm.
 * Result must be DANFOSS_CRC_GOOD.
 *
 * Outgoing: Calculate without the frame (the first byte) over all message bytes
 * (stop befor the checksum bytes). XOR with 0xFFFF. That's your checksum for the protocol.
 *
 * \returns Checksum.
 *
 */
uint16_t CInverterDanfoss::hdlc_calcchecksum(const std::string& input)
    {
    boost::crc_optimal<16, // bits
        0x1021, // polynom (CCITT Polynome)
        0xffff, // initRem
        0x0, // FinalXor
        true, // ReflectIn
        true // ReflectRem
    > crc;

    crc.process_bytes(input.c_str(), input.length());
    return crc.checksum();
}


#endif /* HAVE_INV_DUMMY */
