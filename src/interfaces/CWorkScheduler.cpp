
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <boost/exception/all.hpp>
//#include <boost/exception.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "CWorkScheduler.h"

#include "patterns/ICommand.h"

#include "CTimedWork.h"

#include "semaphore.h"
#include "interfaces/CMutexHelper.h"

using namespace std;

CWorkScheduler::CWorkScheduler()
{
	sem_init(&semaphore, 0, 0);

	// generate thread for the timed work facility.
	timedwork = new CTimedWork(this);
	timedwork->run();
}

CWorkScheduler::~CWorkScheduler()
{
	delete timedwork;
}

bool CWorkScheduler::DoWork( bool block )
{
	if (!block) {
		CMutexAutoLock(&(this->mut));
		if (CommandsDue.empty()) {
			return false;
		}
	}

	sem_wait(&semaphore);
	ICommand *cmd = getnextcmd();
	cmd->execute();
	delete cmd;
	return true;
}

ICommand *CWorkScheduler::getnextcmd( void )
{
	// Obtain Mutex to make sure...
	CMutexAutoLock CMutexHelper(&(this->mut));
	ICommand *cmd = CommandsDue.front();
	CommandsDue.pop_front();
	return cmd;
}

void CWorkScheduler::ScheduleWork( ICommand *Command )
{
	CMutexAutoLock CMutexHelper(&(this->mut));
	CommandsDue.push_back(Command);
	sem_post(&semaphore);
}

void CWorkScheduler::ScheduleWork( ICommand *Command, struct timespec ts )
{
	timedwork->ScheduleWork(Command, ts);
}

