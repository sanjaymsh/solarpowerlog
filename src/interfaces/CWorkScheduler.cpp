

/* ----------------------------------------------------------------------------
   solarpowerlog
   Copyright (C) 2009  Tobias Frost

   This file is part of solarpowerlog.

   Solarpowerlog is free software; However, it is dual-licenced
   as described in the file "COPYING".

   For this file (CWorkScheduler.cpp), the license terms are:

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
/** \file CWorkScheduler.cpp
 *
 *  Created on: May 17, 2009
 *      Author: tobi
 */

#include "CWorkScheduler.h"
#include "CMutexHelper.h"
#include "patterns/ICommand.h"

#include <cc++/thread.h>


// little private class for timed work....
class CTimedWork : public  Thread
{
public:
	CTimedWork(CWorkScheduler *sch, ICommand *cmd, struct timespec ts) {
		this->cmd = cmd;
		this->ts.tv_sec = ts.tv_sec;
		this->ts.tv_nsec = ts.tv_nsec;
		this->sch = sch;
		}

private:
	ICommand *cmd;
	struct timespec ts;
	CWorkScheduler *sch;

protected:
	/// Called on thread termination
	void final()
	{
		// Book out of the list and terminate.
		sch->enterMutex();
		sch->SpawnedThreads.remove(this);
		sch->leaveMutex();
		delete this;
	}

	// Called on execution of the thread.
	void run() {

		setCancel(ost::Thread::cancelImmediate);
		while ( ts.tv_sec || ts.tv_nsec)
		{
			unsigned long t=0;
			// just make sure that we won't overflow the timeout_t
			// we play safe and dont go over 2^24
			if( ts.tv_sec > (1<<24UL))
			{
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

			Thread::sleep(t);
		}

		sch->ScheduleWork(cmd);
	}

};


CWorkScheduler::CWorkScheduler() {
	// TODO Auto-generated constructor stub

}

CWorkScheduler::~CWorkScheduler() {

	do
	{
		this->enterMutex();
		if (SpawnedThreads.empty()) {
			this->leaveMutex();
			return;
		}
		CTimedWork *t = SpawnedThreads.front();
		SpawnedThreads.pop_front();
		this->leaveMutex();
		delete t;
	} while(1);
}


bool CWorkScheduler::DoWork(void)
{
	if ( CommandsDue.empty()) return false;
	ICommand *cmd=getnextcmd();
	cmd->execute();
	delete cmd;

	return true;
}


ICommand *CWorkScheduler::getnextcmd(void)
{
	// Obtain Mutex to make sure...
	CMutexAutoLock CMutexHelper(this);
	ICommand *cmd = CommandsDue.front();
	CommandsDue.pop_front();
	return cmd;
}


void CWorkScheduler::ScheduleWork(ICommand *Command)
{
	CMutexAutoLock h(this);
	CommandsDue.push_back(Command);
}

void CWorkScheduler::ScheduleWork(ICommand *Command, struct timespec ts)
{
	CTimedWork *thread = new CTimedWork(this, Command, ts);
	this->enterMutex();
	SpawnedThreads.push_back(thread);
	this->leaveMutex();
	thread->detach();
}


