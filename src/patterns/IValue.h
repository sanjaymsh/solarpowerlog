/* ----------------------------------------------------------------------------
   solarpowerlog
   Copyright (C) 2009  Tobias Frost

   This file is part of solarpowerlog.

   Solarpowerlog is free software; However, it is dual-licenced
   as described in the file "COPYING".

   For this file (IValue.h), the license terms are:

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


/** \file IValue.h
 *
 *  Created on: May 13, 2009
 *      Author: tobi
 */

#ifndef ICAPABILITY_H_
#define ICAPABILITY_H_

#include <string>

/** A Value is like aconcrete measurements, states, etc.
 *
 * What it makes tricky is. that they might need different data types for storage.
 *
 * TODO DOCUMENT ME!
 */
class IValue {
public:

	enum factory_types
	{
		bool_type,
		int_type,
		float_type,
		string_type
	};

	static IValue* Factory(const factory_types typedescriptor);

	virtual factory_types GetType(void) const;

protected:
	IValue();
public:
	virtual ~IValue();

protected:
	factory_types type;

};


#endif /* ICAPABILITY_H_ */
