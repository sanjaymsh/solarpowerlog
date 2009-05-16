/** \file IValue.cpp
 *
 *  Created on: May 13, 2009
 *      Author: tobi
 */

#include "IValue.h"
#include "CValue.h"

#include <iostream>


using namespace std;

IValue::IValue()
{
}

IValue::factory_types IValue::GetType(void) const
{
	return type;
}

IValue *IValue::Factory(const factory_types newtype)
{
	IValue *tmp;

	switch (newtype)
	{
	case int_type:
		tmp = new CValue<int>;
		break;

	case float_type:
		tmp = new CValue<double>;
		break;

	case string_type:
		tmp = new CValue<std::string>;

	break;
	}

	if (! tmp){
		std::cerr<<"BUG: " << __FILE__ << ":" << __LINE__
			<< " --> Queried for unknown CValue type " << newtype << std::endl;
		return NULL;
	}

	tmp->type = newtype;
	return tmp;

}


IValue::~IValue() {
	// TODO Auto-generated destructor stub
}
