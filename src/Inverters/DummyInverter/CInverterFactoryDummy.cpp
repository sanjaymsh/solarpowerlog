/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009-2011  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CInverterFactoryDummy.cpp), the license terms are:

 You can redistribute it and/or modify it under the terms of the GNU
 General Public License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

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

static const std::string supported_models = "Dummy-Inverter: accepts any model";

CInverterFactoryDummy::CInverterFactoryDummy()
{
}

CInverterFactoryDummy::~CInverterFactoryDummy()
{
}

IInverterBase *CInverterFactoryDummy::Factory(const string &,
		const string & name, const string & configurationpath)
{
	// As this is a dummy, we are not picky and return a object on any model...
	return new CInverterDummy(name, configurationpath);
}

const string & CInverterFactoryDummy::GetSupportedModels() const
{
	return supported_models;
}

#endif /* HAVE_INV_DUMMY */
