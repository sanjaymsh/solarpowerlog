/*
 * InverterBase.cpp
 *
 *  Created on: May 9, 2009
 *      Author: tobi
 */

#include "InverterBase.h"

using namespace std;

IInverterBase::IInverterBase( std::string name ) {
	// TODO Auto-generated constructor stub
	this->name = name;


}

IInverterBase::~IInverterBase() {
	// TODO Auto-generated destructor stub
}


std::string IInverterBase::GetName()
{
	return name;
}


