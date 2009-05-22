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

#include <map>

#include "InverterBase.h"
#include "interfaces/factories/IConnectFactory.h"
#include "interfaces/CCapability.h"

#include "Inverters/Capabilites.h"
#include "patterns/IValue.h"

using namespace std;

IInverterBase::IInverterBase( const string& name, const string & configurationpath  ) {
	this->name = name;
	this->configurationpath = configurationpath;
	connection = IConnectFactory::Factory(configurationpath);

	string s;
	IValue *v;
	CCapability *c;

	// Add the "must have" capabilites.
	s = CAPA_CAPAS_UPDATED_TYPE;
	v = IValue::Factory( CAPA_CAPAS_UPDATED_TYPE );
	c = new CCapability(s , v, this );
	CapabilityMap.insert( pair<string,CCapability*>(s,c));

	// Add the "must have" capabilites.
	s = CAPA_CAPAS_REMOVEALL;
	v = IValue::Factory( CAPA_CAPAS_REMOVEALL_TYPE );
	c = new CCapability(s , v, this );
	CapabilityMap.insert( pair<string,CCapability*>(s,c));

}

IInverterBase::~IInverterBase() {
	if ( connection) delete connection;
	connection = NULL;

	// TODO auto destruct all elements in the capability map.
	map<string, CCapability*>::iterator it = GetCapabilityIterator();
	for ( ; it != GetCapabilityLastIterator() ; it++ )
		    delete (*it).second;
	CapabilityMap.clear();

}


const std::string& IInverterBase::GetName(void) const
{
	return name;
}


