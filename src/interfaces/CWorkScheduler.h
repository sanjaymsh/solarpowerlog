
/* ----------------------------------------------------------------------------
   solarpowerlog
   Copyright (C) 2009  Tobias Frost

   This file is part of solarpowerlog.

   Solarpowerlog is free software; However, it is dual-licenced
   as described in the file "COPYING".

   For this file (CWorkScheduler.h), the license terms are:

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
/** \file CWorkScheduler.h
 *
 *  Created on: May 17, 2009
 *      Author: tobi
 */

#ifndef CWORKSCHEDULER_H_
#define CWORKSCHEDULER_H_

#include <time.h>

class ICommand;
class ICommandTarget;

using namespace std;



/** This class implements the work scheduler:
 *
 * Objects derived from CommandTarget can schedule work to be done:
 *
 * call again later:
 * (when they are not able to complete it immediatly)
 *
 * call again at ....
 * (when they expect to do some work in some specific time
 *
 * \note for all actions, the Objects will be copied and therefore
 * this class takes ownership of the object.
*/
class CWorkScheduler {
public:
	CWorkScheduler();
	virtual ~CWorkScheduler();

	void ScheduleWork(ICommand Command);
	void ScheduleWork(ICommand Commmand, struct timespec ts);


	void DoWork(void);

private:

};

#endif /* CWORKSCHEDULER_H_ */
