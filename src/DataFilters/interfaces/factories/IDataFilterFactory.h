#include "DataFilters/interfaces//IDataFilter.h"
/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (IDataFilterFactory.h), the license terms are:

 You can redistribute it and/or  modify it under the terms of the GNU Lesser
 General Public License (LGPL) as published by the Free Software Foundation;
 either version 3 of the License, or (at your option) any later version.

 This programm is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/** \file IDataFilterFactory.h
 *
 *  Created on: Jun 1, 2009
 *      Author: tobi
 */

#ifndef IDATAFILTERFACTORY_H_
#define IDATAFILTERFACTORY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*fixme COMMENT ME
 *
 *
 * TODO DOCUMENT ME!
 */

class IDataFilter;

class IDataFilterFactory {
public:
	virtual IDataFilter* Factory(const string& type, const string &name, const string &configurationpath);

	IDataFilterFactory();
	virtual ~IDataFilterFactory();
};

#endif /* IDATAFILTERFACTORY_H_ */
