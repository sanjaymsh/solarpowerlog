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

/** \file ICapaIterator.cpp
 *
 *  Created on: Jun 2, 2009
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ICapaIterator.h"
#include "configuration/Registry.h"

ICapaIterator::ICapaIterator( IInverterBase *b, IInverterBase *p )
{
	// TODO Cleanup Debug code cerr << " DEBUG: new ICapaInverter b=" << b->GetName();
	//if (p)
	//	cerr << " p=" << p->GetName();
	// cerr << endl;

	SetBase(b);
	parent = p;
}

void ICapaIterator::SetBase( IInverterBase *b )
{
	base = b;
	it = base-> GetCapabilityIterator();
}

bool ICapaIterator::HasNext()
{
	if (it != base->GetCapabilityLastIterator()) {
		//cerr << "DEBUG: Has Next on " << base->GetName() << " "
		//	<< (*it).first << endl;
		return true;
	} else {
		return false;
	}
}

IInverterBase *ICapaIterator::GetBase()
{
	return base;
}

pair<string, CCapability*> ICapaIterator::GetNext()
{

	if (it != base->GetCapabilityLastIterator()) {
		return *(it++);
	}

	LOGDEBUG(Registry::GetMainLogger(),
		"BUG: NOTHING TO RETURN: USE HasNext() PRIOR GETNEXT()");
	assert(false);
	return *it;
}

ICapaIterator::~ICapaIterator()

{
	// TODO Auto-generated destructor stub
}

pair<string, CCapability*> ICapaIterator::GetElement()
{
	// cerr << "DEBUG: Get Element: Returning Element " << (*it).first << endl;
	return *it;
}

