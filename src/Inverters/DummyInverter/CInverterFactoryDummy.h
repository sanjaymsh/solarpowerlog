/*
 * CInverterFactoryDummy.h
 *
 * This is the factory for the dummy Inverter.
 *
 *  Created on: 17.07.2011
 *      Author: coldtobi
 */

#ifndef CINVERTERFACTORYDUMMY_H_
#define CINVERTERFACTORYDUMMY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_INV_DUMMY

#include "Inverters/factories/IInverterFactory.h"

class CInverterFactoryDummy: public IInverterFactory
{
public:
	CInverterFactoryDummy();
	virtual ~CInverterFactoryDummy();

	virtual IInverterBase * Factory(const string& type, const string& name, const string & configurationpath);

	virtual const string &  GetSupportedModels() const;


};

#endif /* HAVE_INV_DUMMY */

#endif /* CINVERTERFACTORYDUMMY_H_ */
