/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (IConnectFactory.h), the license terms are:

 You can redistribute it and/or  modify it under the terms of the GNU Lesser
 General Public License (LGPL) as published by the Free Software Foundation;
 either version 3 of the License, or (at your option) any later version.

 This programm is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/** \file IConnectFactory.h
 *
 *  \date Created on: May 16, 2009
 *  \author: Tobias Frost (coldtobi)
 */

#ifndef CONNECTIONFACTORY_H_
#define CONNECTIONFACTORY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Connections/interfaces/IConnect.h"

using namespace std;

/** Factory for IConnection-Classes
 *
 *\ingroup factories
 */
class IConnectFactory
{
public:
	/** Facortry for generation of connection methods.
	 * Give it the configurationpath, and out of the config, it will
	 * generate the right class.
	 *
	 * If the class is not known, it will return a dummy connection class.
	 * So you can also create inverters or derived classes without commms.
	 *
	 * \parameter configurationpath Where to extract the type of the
	 * requested comms?
	 *  */
	static IConnect* Factory( const string &configurationpath );

protected:
	IConnectFactory()
	{};
public:
	virtual ~IConnectFactory()
	{};

};

#endif /* CONNECTIONFACTORY_H_ */
