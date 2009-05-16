/*
 * InverterBase.cpp
 *
 *  Created on: May 9, 2009
 *      Author: tobi
 */

#include "InverterBase.h"

using namespace std;

IInverterBase::IInverterBase( std::string name ) {
	this->name = name;
}

IInverterBase::~IInverterBase() {
	// TODO Auto-generated destructor stub
}


const std::string& IInverterBase::GetName(void) const
{
	return name;
}


