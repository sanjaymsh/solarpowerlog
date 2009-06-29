/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licensed
 as described in the file "COPYING".

 For this file (CCSVOutputFilter.h), the license terms are:

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

/** \file CCSVOutputFilter.h
 *
 *  Created on: Jun 29, 2009
 *      Author: Tobias Frost (coldtobi)
 */

#ifndef CCSVOUTPUTFILTER_H_
#define CCSVOUTPUTFILTER_H_

/** \fixme COMMENT ME
 *
 *
 * TODO DOCUMENT ME!
 */
#include "DataFilters/interfaces/IDataFilter.h"

class CCSVOutputFilter : public IDataFilter
{
public:

	CCSVOutputFilter( const string &name, const string & configurationpath );

	virtual ~CCSVOutputFilter();

	virtual bool CheckConfig();

	virtual void  Update( const IObserverSubject *subject );
};

#endif /* CCSVOUTPUTFILTER_H_ */
