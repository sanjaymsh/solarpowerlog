/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CNestedCapaIterator.cpp), the license terms are:

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

/** \file CNestedCapaIterator.cpp
 *
 *  Created on: Jun 2, 2009
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Inverters/interfaces/CNestedCapaIterator.h"

CNestedCapaIterator::CNestedCapaIterator( IInverterBase *b,
	IInverterBase* parent ) :
	ICapaIterator(b, parent)
{
	topmost = b;
	// TODO Auto-generated constructor stub
}

CNestedCapaIterator::~CNestedCapaIterator()
{
	// TODO Auto-generated destructor stub
}

/// Check if we have a next capa, if not, delegate to
/// the parent, if exists.
/// Set also the grandparent, if necessary to recurse all the way down.
///
/// \warning do not use GetNext() without a HasNext()!
bool CNestedCapaIterator::HasNext()
{
	CCapability *c;
	pair<string, CCapability*> p;
	if (ICapaIterator::HasNext()) {
		p = GetElement();
		// Check if we had to filter that one out.
		if (p.second == (c = topmost->GetConcreteCapability(p.first))) {
			// yes, not filtered.
			return true;
		} else {
			// that one is to be filtered.
			// Filter and then recurse.
			ICapaIterator::GetNext();
			return HasNext();
		}
	}
	// appearantly, the curent base is out of capabilities.
	// descent one level deeper.

	if (parent) {
		// Set the parent as new base, and ask the parent for its parent
		// via getting the informations in its iterator.
		this->SetBase(parent);
		ICapaIterator *p = parent->GetCapaNewIterator();
		this->setParent(p->getParent());
		delete p;
	} else {
		return false;
	}
	return ICapaIterator::HasNext();
}

/// One could just delegate to the base class, but
/// we want to make sure that we won't pass
/// capas we have overriden
/// \warning do not use GetNext() without a prior HasNext()!
pair<string, CCapability*> CNestedCapaIterator::GetNext()
{
	// HasNext() already set up everything we need to know...
	return ICapaIterator::GetNext();
}
