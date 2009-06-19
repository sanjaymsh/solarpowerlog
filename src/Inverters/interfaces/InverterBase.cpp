/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (InverterBase.cpp), the license terms are:

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

/*
 * InverterBase.cpp
 *
 *  Created on: May 9, 2009
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <map>
#include <assert.h>

#include "Inverters/interfaces/InverterBase.h"
#include "Connections/factories/IConnectFactory.h"
#include "interfaces/CCapability.h"

#include "Inverters/Capabilites.h"
#include "patterns/IValue.h"

#include "Inverters/interfaces/ICapaIterator.h"

using namespace std;

IInverterBase::IInverterBase( const string& name,
	const string & configurationpath )
{
	this->name = name;
	this->configurationpath = configurationpath;
	connection = IConnectFactory::Factory(configurationpath);
	pair<map<string, CCapability*>::iterator, bool> b;

	string s;
	IValue *v;
	CCapability *c;

	// Add the "must have" capabilites.
	s = CAPA_CAPAS_UPDATED;
	v = IValue::Factory(CAPA_CAPAS_UPDATED_TYPE);
	c = new CCapability(s, v, this);
	b = CapabilityMap.insert(pair<string, CCapability*> (s, c));
	assert( b.second );

	// Add the "must have" capabilites.
	s = CAPA_CAPAS_REMOVEALL;
	v = IValue::Factory(CAPA_CAPAS_REMOVEALL_TYPE);
	c = new CCapability(s, v, this);
	b = CapabilityMap.insert(pair<string, CCapability*> (s, c));
	assert( b.second );

	// TODO remove debug code: next line
	GetCapabilityIterator();

}

IInverterBase::~IInverterBase()
{
	if (connection)
		delete connection;
	connection = NULL;

	// TODO auto destruct all elements in the capability map.
	map<string, CCapability*>::iterator it = GetCapabilityIterator();
	for (; it != GetCapabilityLastIterator(); it++)
		delete (*it).second;
	CapabilityMap.clear();

}

const std::string& IInverterBase::GetName( void ) const
{
	return name;
}

ICapaIterator *IInverterBase::GetCapaNewIterator()
{
	return new ICapaIterator(this);
}

