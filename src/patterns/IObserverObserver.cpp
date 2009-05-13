/** \file IObserverObserver.cpp
 *
 *  Created on: May 12, 2009
 *      Author: tobi
 *
 *  This files implements the Observer for the Observer Design Pattern.
 */



#include "IObserverObserver.h"
#include "IObserverSubject.h"

using namespace std;

/** Constructor for the Observer (Observer Pattern)
 *
 * The Observer will auto-susbscribe to the Subject, if the
 * parameter is supplied. (But what should do a observer without an subjct?)
*/
IObserverObserver::IObserverObserver(IObserverSubject *subject) {

	/* auto-subscribe */
	setSubject(subject);
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
