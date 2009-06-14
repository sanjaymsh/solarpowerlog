/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CMutexAutoLock.h), the license terms are:

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

/** \file CMutexAutoLock.h
 *
 *  Created on: May 17, 2009
 *      Author: tobi
 */

#ifndef CMUTEXHELPER_H_
#define CMUTEXHELPER_H_

#include <cc++/thread.h>

using namespace ost;

/** Mutex Wrapper for the GNU Common C++ Library
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

	CMutexAutoLock(class Mutex* mutex);
	virtual ~CMutexAutoLock();

private:
	class Mutex *mutex;

};

#endif /* CMUTEXHELPER_H_ */
