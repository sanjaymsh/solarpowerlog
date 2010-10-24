/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CNestedCapaIterator.h), the license terms are:

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

