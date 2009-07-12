/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (IValue.cpp), the license terms are:

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

/** \file IValue.cpp
 *
 *  Created on: May 13, 2009
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "IValue.h"
#include "CValue.h"

#include <iostream>
#include <assert.h>

using namespace std;

IValue::IValue()
{
}

IValue::factory_types IValue::GetType( void ) const
{
	return type;
}

IValue *IValue::Factory( const factory_types newtype )
{
	IValue *tmp;

	switch (newtype) {

	case bool_type:
		tmp = new CValue<bool> ;
		((CValue<bool>*) tmp) ->Set(false);
		break;

	case int_type:
		tmp = new CValue<int> ;
		((CValue<int>*) tmp) ->Set(0);
		break;

	case float_type:
		tmp = new CValue<float> ;
		((CValue<float>*) tmp) ->Set(0.0);
		break;

	case string_type:
		tmp = new CValue<std::string> ;
		((CValue<std::string>*) tmp)->Set("");
		break;

	default:
		assert(false);
		break;
	}

	if (!tmp) {
		std::cerr << "BUG: " << __FILE__ << ":" << __LINE__
			<< " --> Queried for unknown CValue type " << newtype
			<< std::endl;
		return NULL;
	}

	tmp->type = newtype;
	return tmp;
}

IValue::~IValue()
{
	// TODO Auto-generated destructor stub
}
