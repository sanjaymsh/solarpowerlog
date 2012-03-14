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

/** \file IObserverSubject.h
 *
 *  Created on: May 12, 2009
 *      Author: tobi
 */

#ifndef OBSERVERSUBJECT_H_
#define OBSERVERSUBJECT_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <list>

using namespace std;

class IObserverObserver;

/** \fixme COMMENT ME
 *
 *
 * TODO DOCUMENT ME!
 */
class IObserverSubject
{
public:

	virtual ~IObserverSubject();

	virtual void Subscribe( class IObserverObserver* observer );

	virtual void UnSubscribe( class IObserverObserver* observer );

	virtual void SetSubscription( class IObserverObserver* observer,
		bool subscribe = true );

	virtual bool CheckSubscription( class IObserverObserver *observer );

	virtual void Notify( void );

	virtual unsigned int GetNumSubscribers( void );

protected:
	IObserverSubject();

private:
	std::list<IObserverObserver*> listobservers;

};

#endif /* OBSERVERSUBJECT_H_ */

/*
 Infos on pattern
 See:
 http://www.cs.clemson.edu/~malloy/courses/patterns/observerCo.html
 */
