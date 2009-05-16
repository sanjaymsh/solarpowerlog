/** \file IObserverSubject.h
 *
 *  Created on: May 12, 2009
 *      Author: tobi
 */

#ifndef OBSERVERSUBJECT_H_
#define OBSERVERSUBJECT_H_

#include <list>

using namespace std;

class IObserverObserver;

/** \fixme COMMENT ME
 *
 *
 * TODO DOCUMENT ME!
 */
class IObserverSubject {
public:

	virtual ~IObserverSubject();

	virtual void Subscribe(class IObserverObserver* observer);

	virtual void UnSubscribe(class IObserverObserver* observer);

	virtual void Notify(void);

	virtual unsigned int GetNumSubscribers(void);

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
