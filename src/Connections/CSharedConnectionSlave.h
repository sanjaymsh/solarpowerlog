/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009-2011  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licensed
 as described in the file "COPYING".

 For this file (CConnectSlave.h), the license terms are:

 You can redistribute it and/or  modify it under the terms of the GNU Lesser
 General Public License (LGPL) as published by the Free Software Foundation;
 either version 3 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/** \file CConnectSlave.h
 *
 * This is the slave object for the Shared Communication.Please see the
 * CSharedConnection class for documentation...
 *
 * (In few words, this will will proxy requests over to amaster object
 * which then will do the comms)
 *
 *  Created on: Sep 13, 2010
 *      Author: coldtobi
 */

#ifndef CCONNECTSLAVE_H_
#define CCONNECTSLAVE_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_COMMS_SHAREDCONNECTION

#include "interfaces/IConnect.h"
#include "CSharedConnectionMaster.h"

class CSharedConnectionSlave: public IConnect
{
protected:
	friend class CSharedConnection;
	CSharedConnectionSlave(const string & configurationname);
public:
	virtual ~CSharedConnectionSlave();

protected:

	virtual void Connect(ICommand *callback);

	virtual void Disconnect(ICommand *callback);

	virtual void Send(ICommand *cmd);

	virtual void Receive(ICommand *callback);

	virtual bool CheckConfig(void);

	virtual bool IsConnected(void);

private:
	class CSharedConnection *master;

};

#endif

#endif /* CCONNECTSLAVE_H_ */
