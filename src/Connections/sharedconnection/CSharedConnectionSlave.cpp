/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2010-2012 Tobias Frost

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
 * CConnectSlave.cpp
 *
 *  Created on: Sep 13, 2010
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_COMMS_SHAREDCONNECTION

#include "CSharedConnectionSlave.h"
#include "CSharedConnectionMaster.h"
#include "configuration/CConfigHelper.h"
#include "Inverters/interfaces/InverterBase.h"
#include "CSharedConnection.h"
#include "configuration/ILogger.h"

// Slave Configuration Parameter
// useconnection = "name"  Name of the inverter having the master connection.

CSharedConnectionSlave::CSharedConnectionSlave(const string & configurationname) :
	IConnect(configurationname)
{
	master = NULL;
	current_ticket = 0; // 0 == no ticket assigned,
}

CSharedConnectionSlave::~CSharedConnectionSlave()
{
}


void CSharedConnectionSlave::Connect(ICommand *callback)
{
    LOGDEBUG(logger,"CSharedConnectionSlave::Connect: callback:"<<callback);
    assert(master);
    assert(callback);
    (void)HandleTickets(callback);
    master->Connect(callback, this);
}

void CSharedConnectionSlave::Disconnect(ICommand *callback)
{
    LOGDEBUG(logger,"CSharedConnectionSlave::Disconnect: callback:"<<callback);
    assert(callback);
    (void)HandleTickets(callback);
    master->Disconnect(callback, this);
}

void CSharedConnectionSlave::Send(ICommand *callback)
{
    LOGDEBUG(logger,"CSharedConnectionSlave::Send: callback:"<<callback);
    assert(callback);
    assert(master);
    (void)HandleTickets(callback);
    master->Send(callback, this);
}

void CSharedConnectionSlave::Receive(ICommand *callback)
{
#warning receive handling needs to be revised to support non-atomic blocks.
    LOGDEBUG(logger,"CSharedConnectionSlave::Receive: callback:"<<callback);
    assert(master);
    assert(callback);

    if (HandleTickets(callback)) {
        master->Receive(callback, this);
        // Receive within an atomic block.
    } else {
        // non-atomic reads are not yet supported!!!
        // (slave side support missing: timeout handling and master-side buffer
        // sharing)
        // lets abort() for now.
        assert(!"not implemented");
    }
}

void CSharedConnectionSlave::Accept(ICommand* callback)
{
#warning not implemented.
    assert(callback);
    assert(!"not implemented");
}

void CSharedConnectionSlave::Noop(ICommand* cmd)
{
    LOGDEBUG(logger,"CSharedConnectionSlave::Noop: callback:"<<cmd);
    assert(cmd);
    (void)HandleTickets(cmd);
    master->Noop(cmd, this);
}

bool CSharedConnectionSlave::CheckConfig(void)
{

	CConfigHelper cfg(ConfigurationPath);
	bool fail = false;
	std::string s;
	fail |= !cfg.GetConfig("useconnection", s);

	fail |= !cfg.CheckConfig("useconnection", libconfig::Setting::TypeString,
			false);

	if (fail)
		return false;

	// Retrieve the pointer to the CSharedConnectionMaster via the Inverter,
	// but do checks to ensure that the type is right.
	IInverterBase *base = Registry::Instance().GetInverter(s);

	// Check if inverter is known
	if (!base) {
		LOGERROR(logger,"useconnection must point to a known Inverter and this "
				"inverter must be declared first. Inverter not found: " << s;);
		return false;
	}

	// Check if the config of this inverter is a shared comm.
	CConfigHelper bcfg(base->GetConfigurationPath());
	bcfg.GetConfig("comms", s, std::string(""));
	if (s != "SharedConnection") {
		LOGERROR(logger,"inverter " << base->GetName() <<
				" does not use a shared connection.");
		return false;
	}

	// Make sure it is a master.
	bcfg.GetConfig("sharedconnection_type", s);
	if (s != "master") {
		LOGERROR(logger,"inverter " << base->GetName() <<
				" does not use a shared master connection" );
		return false;
	}

	// The API of the SharedConnection gives us a IConnect*, promote it to
	// (CSharedConnectionMaster* (we checked the type above...)
    master =
        (CSharedConnectionMaster*)((CSharedConnection*)base->getConnection())
            ->GetConcreteSharedConnection();
	LOGDEBUG(logger, ConfigurationPath << " slave:" << this << " master " << master);
	return true;
}

bool CSharedConnectionSlave::IsConnected(void)
{
	assert(master);
	return master->IsConnected();
}

bool CSharedConnectionSlave::AbortAll()
{
#warning code for non-atomic operations.
    return false;
}


bool CSharedConnectionSlave::HandleTickets(ICommand* callback)
{
    // get if this block goes atomic or ends an atomic block.
    bool is_still_atomic = false;
    bool is_atomic = false;
    try {
        is_still_atomic = boost::any_cast<bool>(
            callback->findData(ICONN_ATOMIC_COMMS));
        is_atomic = true;
    } catch (const std::invalid_argument &e) {
        // data not contained -- it is a non-atomic block.
        is_atomic = false;
    } catch (const boost::bad_any_cast &e) {
        //"Bad cast -- Programming error."
        //throw; // abort programm.
    }

    if (!is_atomic) {
        LOGDEBUG(logger," Not atomic " << callback);
        return false;
    }

    // New atomic block?
    if (0 == current_ticket) {
        current_ticket = master->GetTicket();
        LOGDEBUG(logger," New ticket "<< current_ticket << " requested for " << callback);
    } else  {
       if ( is_still_atomic) LOGDEBUG(logger," Existing ticket "<< current_ticket << " continued for " << callback);
       else  LOGDEBUG(logger," Existing ticket "<< current_ticket << " ending with " << callback);
    }

    callback->addData(ICONN_SHARED_TICKET, current_ticket);

    // is this the last astomic request?
    if (!is_still_atomic) current_ticket = 0;
    return true;
}

#endif
