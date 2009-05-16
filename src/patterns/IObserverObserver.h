/** \file IObserverObserver.h
 *
 *  Created on: May 12, 2009
 *      Author: tobi
 */

#ifndef OBSERVEROBSERVER_H_
#define OBSERVEROBSERVER_H_

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
    IObserverSubject *getSubject() const;

    void setSubject(IObserverSubject *subject);

private:
	IObserverSubject *subject;

};

#endif /* OBSERVEROBSERVER_H_ */


