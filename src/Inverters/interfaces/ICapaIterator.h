/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (ICapaIterator.h), the license terms are:

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

/** \file ICapaIterator.h
 *
 *  Created on: Jun 2, 2009
 *      Author: tobi
 */

#ifndef ICAPAITERATOR_H_
#define ICAPAITERATOR_H_

#include <string>
#include <map>
#include "Inverters/interfaces/InverterBase.h"

class CCapability;

/** \fixme COMMENT ME
 *
 *
 * TODO DOCUMENT ME!
 */
class ICapaIterator
{
public:
private:
	IInverterBase *base;
	map<string, CCapability*>::iterator it;
protected:
	IInverterBase *parent;
public:
	ICapaIterator( IInverterBase *b, IInverterBase *parent = 0 );

	virtual ~ICapaIterator();

	IInverterBase *GetBase();

	virtual void SetBase( IInverterBase *b );

	virtual bool HasNext();

	virtual pair<string, CCapability*> GetNext();

	virtual pair<string, CCapability*> GetElement();

	// TODO Rename that meethod. It has nothing to do with "the parent element",
	// but the "parent hierachy element", the one-level-deeper....
	IInverterBase *getParent() const
	{
		return parent;
	}

	void setParent( IInverterBase *parent )
	{
		this->parent = parent;
	}

};

#endif /* ICAPAITERATOR_H_ */

