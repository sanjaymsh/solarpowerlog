/* ----------------------------------------------------------------------------
   solarpowerlog
   Copyright (C) 2009  Tobias Frost

   This file is part of solarpowerlog.

   Solarpowerlog is free software; However, it is dual-licenced
   as described in the file "COPYING".

   For this file (IObserverObserver.cpp), the license terms are:

   You can redistribute it and/or modify it under the terms of the GNU
   General Public License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this proramm; if not, see
   <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------------
*/

/** \file IObserverObserver.cpp
 *
 *  Created on: May 12, 2009
 *      Author: tobi
 *
 *  This files implements the Observer for the Observer Design Pattern.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "IObserverObserver.h"
#include "IObserverSubject.h"

using namespace std;

/** Constructor for the Observer (Observer Pattern)
 *
 * The Observer will auto-susbscribe to the Subject, if the
 * parameter is supplied. (But what should do a observer without an subject?)
*/
IObserverObserver::IObserverObserver(IObserverSubject *subject) {

	/* auto-subscribe */
	if (subject != NULL) setSubject(subject);
}

/** The destructor will auto-unsubsribe before destroying the object. */
IObserverObserver::~IObserverObserver() {

	/* auto-unsubsribe */
	if (subject) subject->UnSubscribe(this);
	// TODO Auto-generated destructor stub
}

/** Getter for the current Subject */
IObserverSubject *IObserverObserver::getSubject() const
 {
     return subject;
 }

/** Set a new Subject, subscribe to it.
 *
 * Will also unsubscribe to the old subject, if available.
 *
 * Note: Will do nothing, if current Subject is the same as the new one.
 * */
 void IObserverObserver::setSubject(IObserverSubject *subject)
 {
	 if(this->subject == subject) return;
	 if(this->subject) subject->UnSubscribe(this);
     this->subject = subject;
     subject->Subscribe(this);
 }
