/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2011-2012 Tobias Frost

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
