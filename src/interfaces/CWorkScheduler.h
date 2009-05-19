
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
#include <list>
#include <map>
#include <cc++/thread.h>


class ICommand;
class ICommandTarget;
class CTimedWork;

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
 * \warning for all actions, the Objects will be passed by reference. At this
 * point of time, CWorkScheduler is owner of the object and will destroy it later,
 * when used.
 *
*/
class CWorkScheduler : protected ost::Mutex {

	friend class CTimedWork;

public:
	CWorkScheduler();
	virtual ~CWorkScheduler();

	void ScheduleWork(ICommand *Command);

	/** Schedule a work for later */
	void ScheduleWork(ICommand *Commmand, struct timespec ts);

	/** Call this method to do dispatch due work.
	 * Note: Returns after each piece of work has been done!
	 *
	 * returns true, if work has been done, false, if no work was available.
	*/
	bool DoWork(void);

private:

	list<ICommand*> CommandsDue;

	list<CTimedWork*> SpawnedThreads;

#if 0
	struct timepec_compare
	{
		 bool operator()(const struct timespec t1, const struct timespec t2) const
		  {
			 if(t1.tv_sec < t2.tv_sec) return true;
			 if(t1.tv_sec > t2.tv_sec) return false;
			 if(t1.tv_nsec < t2.tv_nsec) return true;
			 return false;
		  };
	};

	multimap<struct timespec, ICommand*, timepec_compare> TimedCommands;
#endif

	/** get the next new command in the list.
	 * (Thread safe)*/
	ICommand *getnextcmd(void);

};

#endif /* CWORKSCHEDULER_H_ */
