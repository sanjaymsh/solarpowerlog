/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2009-2012 Tobias Frost

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

/** \file CCapability.cpp
 *
 *  \date Created on: May 16, 2009
 *  \author Tobias Frost (coldtobi)
 *
 *  \sa \ref CapaConcept "Capability Concept"
 *  \sa \ref CCapability
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "CCapability.h"

using namespace std;

CCapability::CCapability( const string& descr, IValue *val,
	IInverterBase *datasrc )
{
	description = descr;
	source = datasrc;
	value = val;
}

CCapability::CCapability( const string &descr, IValue::factory_types type,
	IInverterBase *datasrc )
{
	description = descr;
	source = datasrc;
	value = IValue::Factory(type);
}

CCapability::~CCapability()
{
	if (value)
		delete value;
	value = NULL;
}
