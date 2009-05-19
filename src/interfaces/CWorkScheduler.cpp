

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
#include "CTimedWork.h"

using namespace std;
using namespace ost;

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


