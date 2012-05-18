/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2009-2012 Tobias Frost

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ----------------------------------------------------------------------------
 */

/** \file IValue.h
 *
 *  \date May 13, 2009
 *  \Author: Tobias Frost (coldtobi)
 */

#ifndef IVALUE_H_
#define IVALUE_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>

/** IValue is the interface to arbitrary value storage.
 *
 * It is supposed to be derived, and the derived class is responsible for
 * type-correct storage.
 *
 *
 * \ingroup factories
 */
class IValue
{
public:

protected:
    template<class T>
    friend class CValue;
	virtual int GetInternalType( void ) const {return type_;}

public:
	/** Interface method for easier transfer to strings. */
	virtual operator std::string() = 0;

protected:
	IValue() {}
public:
	virtual ~IValue() {}

protected:
	int type_;

};

#endif /* IVALUE_H_ */
