/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2011-2014 Tobias Frost

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ----------------------------------------------------------------------------
*/

/*
 * CInverterFactoryDanfoss.cpp
 *
 *  Created on: 18.05.2014
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_INV_DANFOSS

#include "Inverters/Danfoss/CInverterFactoryDanfoss.h"
#include "Inverters/Danfoss/CInverterDanfoss.h"

static const std::string supported_models = "Danfoss-Inverter: UniLynx TripleLynx";

CInverterFactoryDanfoss::CInverterFactoryDanfoss()
{
}

CInverterFactoryDanfoss::~CInverterFactoryDanfoss()
{
}

IInverterBase *CInverterFactoryDanfoss::Factory(const string &type,
		const string & name, const string & configurationpath)
{
    if ( type == "UniLynx" || type == "TripleLynx" )
    {
        return new CInverterDanfoss(type, name, configurationpath);
    }
	return NULL;
}

const string & CInverterFactoryDanfoss::GetSupportedModels() const
{
	return supported_models;
}

#endif /* HAVE_INV_DUMMY */
