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

/** \file CMutexAutoLock.h
 *
 *  Created on: May 17, 2009
 *      Author: tobi
 */

#ifndef CMUTEXHELPER_H_
#define CMUTEXHELPER_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <boost/thread/mutex.hpp>

/** Mutex Wrapper
 *
 * The library makes you keeping track when you entered the
 * Mutex. This generates the problem, that if the function
 * is left without considering the lock, the lock remains forever.
 *
 * This class helps in this case: Just create an object of it,
 * passing the "this" pointer to the constructor
 * (assuming your class is derived from Mutex) and enjoy the lock.
 *
 * The lock is automatically destroyed on destrucing the object.
 */
class CMutexAutoLock {
public:
	CMutexAutoLock(boost::mutex * mutex);
	virtual ~CMutexAutoLock();

	void unlock(void) {
		if (locked) mutex->unlock();
		locked = false;
	}

	void lock(void) {
		if (!locked) mutex->lock();
		locked = true;
	}

private:
	bool locked;
	boost::mutex *mutex;

};

#endif /* CMUTEXHELPER_H_ */
