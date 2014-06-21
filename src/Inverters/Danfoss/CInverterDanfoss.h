/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

 Copyright (C) 2011-2014 Tobias Frost

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

/*
 * CInverterDanfoss.h
 *
 *  Created on: 14.05.2014
 *      Author: tobi
 */

#ifndef CINVERTERDANFOSS_H_
#define CINVERTERDANFOSS_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_INV_DANFOSS

#include "Inverters/interfaces/InverterBase.h"
#include "Inverters/BasicCommands.h"

#include "Inverters/SputnikEngineering/SputnikCommand/ISputnikCommand.h"

#include <string>

class CInverterDanfoss : public IInverterBase
{
public:
    CInverterDanfoss(const std::string &type, const std::string &name,
        const std::string & configurationpath);

    virtual ~CInverterDanfoss();

    virtual bool CheckConfig();

    virtual void ExecuteCommand(const ICommand *Command);

private:

    // Protocol functions.
    // (might be better of in a dedicated class, though...
    std::string hdlc_debytestuff(const std::string &input);
    std::string hdlc_bytestuff(const std::string &input);

    uint16_t hdlc_calcchecksum(const std::string &input);

    std::string assemblequerystring(void);

    /// parse the answer of the inverter.
    int parsereceivedstring(std::string &rcvd);

    enum Commands
     {
         // broadcast event.
         CMD_BRC_SHUTDOWN = BasicCommands::CMD_BRC_SHUTDOWN,
         CMD_INIT = BasicCommands::CMD_USER_MIN,
         CMD_WAIT4CONNECTION,
         CMD_IDENTFY_WAIT,
         CMD_POLL,
         CMD_DISCONNECTED,
         CMD_DISCONNECTED_WAIT,
         CMD_EVALUATE_RECEIVE,
         CMD_WAIT_SENT,
         CMD_SEND_QUERIES,
         CMD_QUERY_POLL
     };

    /// Inverter type -- passed from factory creating our instance.
    std::string _invertertype;

    /// set to true if the shutdown request has been received via broadcast
    /// event.
    bool _shutdown_requested;

    /// Configuration Cache: Timeout for telegramm, unit is ms
    float _cfg_response_timeout_ms;

    /// Configuration Cache: Timeout to establish a connection, unit ms
    float _cfg_connection_timeout_ms;

    /// Configuration Cache: Timeout to send a telegramm, unit ms
    float _cfg_send_timeout_ms;

    /// Configuration Cache: Reconnect delay, unit s
    float _cfg_reconnectdelay_s;

    /** Address information for the Inverter: Network
     *
     * This var caches a configuation option
     */
    int _cfg_inv_network_adr;

    /** Address information for the Inverter: Subnet
     *
     * This var caches a configuation option
     */
    int _cfg_inv_subnet_adr;

    /** Address information for the Inverter: Address
     *
     * This var caches a configuation option
     */
    int _cfg_inv_adr;

    /** Address information for the Master : Network
     *
     * This var caches a configuation option
     */
    int _cfg_master_network_adr;

    /** Address information for the Master : Subnet
     *
     * This var caches a configuation option
     */

    int _cfg_master_subnet_adr;
    /** Address information for the Master : Address
     *
     * This var caches a configuation option
     */
    int _cfg_master_adr;

    unsigned int _precalc_masteradr;
    unsigned int _precalc_slaveadr;
     // Handling of commands (like on the Sputnik inverter)
    /// stores supported commands.
    vector<ISputnikCommand*> commands;

    /// stores pending commmands: The set from this->commands which has not
    /// been yet queried
    deque<ISputnikCommand*> pendingcommands;

    /** For the Danfoss Inverter (as it only can query one command at at time
     * notansweredcommand contains the command which is currently being queried.
     * NULL if empty.
     */
    ISputnikCommand* _notansweredcommand;

    void _localdebug(void);

};

#endif /* HAVE_INV_DUMMY */

#endif /* CINVERTERDANFOSS_H_ */
