/** \file IObserverSubject.cpp
 *
 *  Created on: May 12, 2009
 *      Author: tobi
 *
 *   This file implements the Subject of the Observer Design Pattern
 */

#include "IObserverSubject.h"
#include "IObserverObserver.h"
#include <list>

using namespace std;


void IObserverSubject::Subscribe(class IObserverObserver *observer)
{
	listobservers.push_back(observer);
}

void IObserverSubject::UnSubscribe(class IObserverObserver *observer)
{
	listobservers.remove(observer);
}


void IObserverSubject::Notify(void)
{
	std::list<class IObserverObserver *>::iterator i;

	for( i=listobservers.begin(); i!=listobservers.end(); ++i )
	{
		(*i)->Update(this);
	}
}


unsigned int IObserverSubject::GetNumSubscribers(void)
{
	return listobservers.size();
}

IObserverSubject::IObserverSubject() {
	// TODO Auto-generated constructor stub

}

IObserverSubject::~IObserverSubject() {
	listobservers.empty();
	// TODO Auto-generated destructor stub
}
