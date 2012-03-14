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

/** \file IObserverObserver.h
 *
 *  Created on: May 12, 2009
 *      Author: tobi
 */

#ifndef OBSERVEROBSERVER_H_
#define OBSERVEROBSERVER_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

using namespace std;

class IObserverSubject;

/** \fixme COMMENT ME
 *
 *
 * TODO DOCUMENT ME!
 */
class IObserverObserver {
public:

	IObserverObserver(IObserverSubject *subject = 0);

	virtual ~IObserverObserver();

	virtual void Update(const class IObserverSubject * subject) = 0;
    virtual IObserverSubject *getSubject() const;

    virtual void setSubject(IObserverSubject *subject);

private:
	IObserverSubject *subject;

};

#endif /* OBSERVEROBSERVER_H_ */


