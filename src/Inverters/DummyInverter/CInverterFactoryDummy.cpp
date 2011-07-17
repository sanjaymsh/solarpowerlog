/*
 * CInverterFactoryDummy.cpp
 *
 *  Created on: 17.07.2011
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_INV_DUMMY

#include "Inverters/DummyInverter/CInverterFactoryDummy.h"
#include "Inverters/DummyInverter/CInverterDummy.h"

CInverterFactoryDummy::CInverterFactoryDummy()
{
}

CInverterFactoryDummy::~CInverterFactoryDummy()
{
}

IInverterBase *CInverterFactoryDummy::Factory(const string & type,
		const string & name, const string & configurationpath)
{
	// As this is a dummy, we are not picky and return a object on any model...
	return new CInverterDummy(name, configurationpath);
}

const string & CInverterFactoryDummy::GetSupportedModels() const
{
	return "Dummy-Inverter: accepts any model";
}

#endif /* HAVE_INV_DUMMY */
