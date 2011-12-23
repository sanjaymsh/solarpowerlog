/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009-2011  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licensed
 as described in the file "COPYING".

 For this file (CFormatterSearchCSVEntry.h), the license terms are:

 You can redistribute it and/or  modify it under the terms of the GNU Lesser
 General Public License (LGPL) as published by the Free Software Foundation;
 either version 3 of the License, or (at your option) any later version.

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
 * CFormatterSearchCSVEntry.h
 *
 *  Created on: Jan 5, 2010
 *      Author: tobi
 */

#ifndef CFORMATTERSEARCHCSVENTRY_H_
#define CFORMATTERSEARCHCSVENTRY_H_

#include "DataFilters/HTMLWriter/formatter/IFormater.h"

class CFormatterSearchCSVEntry: public IFormater
{

public:
	virtual ~CFormatterSearchCSVEntry();

	virtual bool Format(const std::string &what, std::string &store,
			const std::vector<std::string> &parameters);

protected:
	// make the factory class my friend to instanciate the object.
	friend class IFormater;
	CFormatterSearchCSVEntry();

};

#endif /* CFORMATTERSEARCHCSVENTRY_H_ */
