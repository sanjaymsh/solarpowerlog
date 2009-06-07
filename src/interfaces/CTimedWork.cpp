/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CTimedWork.cpp), the license terms are:

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

/** \file CTimedWork.cpp
 *
 *  Created on: May 18, 2009
 *      Author: tobi
 */

#include <iostream>
#include <list>
#include <cc++/thread.h>

#include "CTimedWork.h"


CTimedWork::CTimedWork(CWorkScheduler *sch, ICommand *cmd, struct timespec ts) {
	this->cmd = cmd;
	this->ts.tv_sec = ts.tv_sec;
	this->ts.tv_nsec = ts.tv_nsec;
	this->sch = sch;
}


void CTimedWork::final()
{
	// Book out of the list and terminate.
	sch->enterMutex();
	sch->SpawnedThreads.remove(this);
	sch->leaveMutex();
//	delete this;  <-- mmmh, leads to crash....
}

// Called on execution of the thread.
void CTimedWork::run() {
	setCancel(ost::Thread::cancelImmediate);
	while ( ts.tv_sec || ts.tv_nsec)
	{
#if 0
		unsigned long t=0;
		// just make sure that we won't overflow the timeout_t
		// we play safe and dont go over 2^24
		if( ts.tv_sec > (1<<24UL))
		{
			// (unlikley that you want to wait more than 2^24 secs...)
			t = (1UL << 24UL) * 1000UL;
			ts.tv_sec -=(1UL << 24UL);
		}
		else
		{
			t = ts.tv_sec * 1000;
		}

		t += (ts.tv_nsec / (1000*1000));
		ts.tv_nsec = 0;

		// Ensure minimum sleep.
		if(!t) {
			// TODO Issue a "BUG" here.
			break;
		}

		this->sleep(t);

#else
		if ( 0 == nanosleep(&ts, &ts)) break;
#endif
	}
	// cout << "TIMEDWORK DONE " << endl;
	sch->ScheduleWork(cmd);
	exit();
}
