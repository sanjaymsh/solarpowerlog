/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2009-2012 Tobias Frost

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

/** \file IInverterFactory.h
 *
 *  Created on: May 20, 2009
 *      Author: tobi
 */

#ifndef IINVERTERFACTORY_H_
#define IINVERTERFACTORY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

class IInverterBase;

#include <string>

using namespace std;

/** \fixme COMMENT ME
 *
 *
 * TODO DOCUMENT ME!
 */
class IInverterFactory {
public:

	virtual IInverterBase * Factory(const string& type, const string& name, const string & configurationpath) = 0;

	virtual const string &  GetSupportedModels() const = 0;

	IInverterFactory();
	virtual ~IInverterFactory();
};

#endif /* IINVERTERFACTORY_H_ */
