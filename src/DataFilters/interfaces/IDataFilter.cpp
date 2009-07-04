/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (IDataFilter.cpp), the license terms are:

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

/** \file IDataFilter.cpp
 *  \date Jun 1, 2009
 *  \author Tobias Frost (coldtobi)
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "DataFilters/interfaces/IDataFilter.h"
#include "Inverters/interfaces/CNestedCapaIterator.h"

IDataFilter::~IDataFilter()
{
	// TODO Auto-generated destructor stub
}

ICapaIterator *IDataFilter::GetCapaNewIterator()
{
	return new CNestedCapaIterator(this, base);
}

CCapability *IDataFilter::GetConcreteCapability( const string & identifier )
{
	CCapability *c;
	if ((c = IInverterBase::GetConcreteCapability(identifier))) {
		// TODO cleanup debug code
		// cout << "DEBUG: found " << identifier << " in " << GetName()
		//	<< endl;
		return c;
	} else {
		// TODO cleanup debug code
		//cout << "DEBUG: searching " << identifier << " in base class "
		//	<< base->GetName() << endl;
		return base->GetConcreteCapability(identifier);
	}
}

