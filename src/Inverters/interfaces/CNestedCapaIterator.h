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

/** \file CNestedCapaIterator.h
 *
 *  Created on: Jun 2, 2009
 *      Author: tobi
 */

#ifndef CNESTEDCAPAITERATOR_H_
#define CNESTEDCAPAITERATOR_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/** \fixme COMMENT ME
 *
 *
 * TODO DOCUMENT ME!
 */
#include "Inverters/interfaces/ICapaIterator.h"

class CNestedCapaIterator : public ICapaIterator
{
public:
	CNestedCapaIterator(IInverterBase *b, IInverterBase *parent = NULL);

	virtual bool HasNext();

	virtual pair<string,CCapability*> GetNext();

	virtual ~CNestedCapaIterator();

private:
	IInverterBase *topmost;

};

#endif /* CNESTEDCAPAITERATOR_H_ */

