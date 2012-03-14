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

/** \file CValue.h
 *
 *  Created on: May 14, 2009
 *      Author: tobi
 *
 * Template-Class for the concrete Values.
 *
 * Note: The factory has to set IValue::type, as I don't know how to...
 */

#ifndef CVALUEX_H_
#define CVALUEX_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "IValue.h"

#include <iostream>
#include <sstream>

template<class T>
class CValue : public IValue
{

public:
	CValue()
	{
	}

	void Set( T value )
	{
		this->value = value;
	}

	T Get( void ) const
	{
		return value;
	}

	virtual void operator=( const T& val )
	{
		value = val;
	}

	virtual void operator=( const CValue<T> &val )
	{
		value = val.Get();
	}

	virtual operator std::string()
	{
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

private:
	T value;

};

#endif /* CVALUEX_H_ */
