/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (IDataFilterFactory.cpp), the license terms are:

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

/** \file IDataFilterFactory.cpp
 *
 *  \date Jun 1, 2009
 *  \author Tobias Frost (coldtobi)
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "DataFilters/interfaces/factories/IDataFilterFactory.h"
#include "DataFilters/CDumpOutputFilter.h"

#include "configuration/Registry.h"

IDataFilter *IDataFilterFactory::Factory(const string & configurationpath)
{

	libconfig::Setting & set = Registry::Instance().GetSettingsForObject(
		configurationpath);
	string type, name;

	set.lookupValue("type", type);
	set.lookupValue("name", name);

	if (type == "DumbDumper") {
		return new CDumpOutputFilter(name, configurationpath);
	}

	return NULL;
}
