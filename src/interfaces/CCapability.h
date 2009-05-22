/* ----------------------------------------------------------------------------
   solarpowerlog
   Copyright (C) 2009  Tobias Frost

   This file is part of solarpowerlog.

   Solarpowerlog is free software; However, it is dual-licenced
   as described in the file "COPYING".

   For this file (CCapability.h), the license terms are:

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

/** \file CCapability.h
 *
 *  Created on: May 16, 2009
 *      Author: tobi
 */

#ifndef CCAPABILITY_H_
#define CCAPABILITY_H_

#include <string>
#include "patterns/IObserverSubject.h"

class IInverterBase;
class IValue;

using namespace std;

/** \fixme COMMENT ME
 *
 *
 * TODO DOCUMENT ME!
 */
class CCapability : public IObserverSubject {
public:
	CCapability( const string& descr,  IValue *val, IInverterBase *datasrc = 0);
	virtual ~CCapability();

	/* Capabilty itself */

	/// Describing the capabilty
	string description;

	/// Where has this data originated. (back-link for the curious object)
	/// Filters propagate this information.
	IInverterBase *source;

	/// Storing the associated value. Please note that this class does not know
	/// anything about its type.
	IValue *value;

	/* Capability as a Subject: Everthing already set!*/

};

#endif /* CCAPABILITY_H_ */
