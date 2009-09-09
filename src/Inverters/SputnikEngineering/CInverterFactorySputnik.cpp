/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CInverterFactorySputnik.cpp), the license terms are:

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

/** \file CInverterFactorySputnik.cpp
 *
 *  Created on: May 20, 2009
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Inverters/SputnikEngineering/CInverterFactorySputnik.h"
#include "Inverters/SputnikEngineering/CInverterSputnikSSeries.h"

using namespace std;

#if defined HAVE_INV_SPUTNIK

static const string supportedmodels =
	"S-Series: \tModels 2000S, 3000S, 4200S, 6000S \n ";

CInverterFactorySputnik::CInverterFactorySputnik() {


// TODO Auto-generated constructor stub

}

/** Instanciates the right inverter class */
IInverterBase *CInverterFactorySputnik::Factory(const string & type,
		const string& name, const string & configurationpath)
{

	if ( type == "S-Series" )
	{
		return new CInverterSputnikSSeries(name, configurationpath);
	}

	return NULL;
}


CInverterFactorySputnik::~CInverterFactorySputnik() {
	// TODO Auto-generated destructor stub
}

/** Return a string describing the available models for this factory.
 * This should be inforamative for the user. */
const string & CInverterFactorySputnik::GetSupportedModels() const {
	return supportedmodels;
}

#endif
