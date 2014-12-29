/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2009-2012 Tobias Frost

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

/** \file CConnectDummy.h
 *
 *  Created on: May 22, 2009
 *      Author: tobi
 */

#ifndef CCONNECTDUMMY_H_
#define CCONNECTDUMMY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#include "configuration/Registry.h"

/** This is a dummy connection class which only fills the gap if
 * - the instanciator does not need comms  but the
 * base class wants a instance. (e.g datafilters)
 */
#include "interfaces/IConnect.h"

class CConnectDummy: public IConnect {

protected:
	friend class IConnectFactory;
	CConnectDummy(const string &configurationname);

public:
	virtual ~CConnectDummy();

private:
	/** The dummy inverter cannot do anything, so we will just
	 * return an error on every request.
	 * this function does that using the async interface.
	 * NOTE: It is a BUG if any of those routines are called, as th
	 * CConnectDummy must not be used as communication interface.
	 * If a user configured it, the config check of it will fail,
	 * aborting the programm.
	*/
	virtual void Dispatch_Error(ICommand *cmd)
	{
		assert(cmd);
		cmd->addData(ICMD_ERRNO, -EIO);
		cmd->addData(ICMD_ERRNO_STR,std::string("CConnectDummy cannot communicate"));
		Registry::GetMainScheduler()->ScheduleWork(cmd);
	}

public:

    virtual void Connect(ICommand *cmd) {
        this->Dispatch_Error(cmd);
    }

    virtual void Disconnect(ICommand *cmd) {
        this->Dispatch_Error(cmd);
    }

    virtual void Send(ICommand *cmd) {
        Dispatch_Error(cmd);
    }

    virtual void Receive(ICommand *cmd) {
        return this->Dispatch_Error(cmd);
    }

    virtual bool AbortAll() {
        return false;
    }

    virtual bool CheckConfig(void);

    virtual void _main(void) {}

    virtual bool CanAccept(void) {
        return false;
    }

};

#endif /* CCONNECTDUMMY_H_ */
