/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (IObserverSubject.cpp), the license terms are:

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

/** \file IObserverSubject.cpp
 *
 *  Created on: May 12, 2009
 *      Author: tobi
 *
 *   This file implements the Subject of the Observer Design Pattern
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "IObserverSubject.h"
#include "IObserverObserver.h"
#include <list>

using namespace std;

void IObserverSubject::Subscribe( class IObserverObserver *observer )
{
	listobservers.push_back(observer);
}

void IObserverSubject::UnSubscribe( class IObserverObserver *observer )
{
	listobservers.remove(observer);
}

void IObserverSubject::Notify( void )
{
	std::list<class IObserverObserver *>::iterator i;
	for (i = listobservers.begin(); i != listobservers.end(); ++i) {
		(*i)->Update(this);
	}
}

unsigned int IObserverSubject::GetNumSubscribers( void )
{
	return listobservers.size();
}

bool IObserverSubject::CheckSubscription( class IObserverObserver *observer )
{
	std::list<class IObserverObserver *>::iterator i;
	for (i = listobservers.begin(); i != listobservers.end(); ++i) {
		if ((*i) == observer)
			return true;
	}

	return false;
}

void IObserverSubject::SetSubsubscription( class IObserverObserver *observer,
	bool subscribe )
{
	if (subscribe)
		Subscribe(observer);
	else
		UnSubscribe(observer);
}

IObserverSubject::IObserverSubject()
{
	// TODO Auto-generated constructor stub

}

IObserverSubject::~IObserverSubject()
{
	listobservers.empty();
	// TODO Auto-generated destructor stub
}
