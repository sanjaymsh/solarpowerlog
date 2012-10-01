/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2009-2012 Tobias Frost

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "CWorkScheduler.h"

#include "patterns/ICommand.h"

#include "CTimedWork.h"

#include "semaphore.h"
#include "interfaces/CMutexHelper.h"

#include "patterns/ICommandTarget.h"

using namespace std;

CWorkScheduler::CWorkScheduler()
    : dhc("CWorkScheduler")
{

    broadcast_subscribers = NULL;

    works_received=0;
    works_completed=0;
    works_timed_scheduled = 0;

    dhc.Register(new CDebugObject<void*>("instance",this));
    dhc.Register(new CDebugObject<int>("works_received",works_received));
    dhc.Register(new CDebugObject<int>("works_completed",works_completed));
    dhc.Register(new CDebugObject<int>("works_timed_scheduled",works_timed_scheduled));

    sem_init(&semaphore, 0, 0);

	// generate thread for the timed work facility.
	timedwork = new CTimedWork(this);

	timedwork->run();
}

CWorkScheduler::~CWorkScheduler()
{
	delete timedwork;
	if (broadcast_subscribers) delete broadcast_subscribers;
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
	if ( cmd->getCmd() > CMD_BROADCAST_MAX) {
	    cmd->execute();
	} else {
	    if (broadcast_subscribers) {
            std::set<ICommandTarget*>::iterator it;
            for (it = broadcast_subscribers->begin(); it != broadcast_subscribers->end(); it++) {
                (*it)->ExecuteCommand(cmd);
            }
	    }
	}
	delete cmd;
	return true;
}

void CWorkScheduler::RegisterBroadcasts(ICommandTarget* target,
    bool subscribe) {

    if (!broadcast_subscribers) {
        broadcast_subscribers = new std::set<ICommandTarget*>;
    }

    if (subscribe) {
        broadcast_subscribers->insert(target);
    }
    else {
        broadcast_subscribers->erase(target);
    }
}

ICommand *CWorkScheduler::getnextcmd( void )
{
	// Obtain Mutex to make sure...
	this->mut.lock();
	ICommand *cmd = CommandsDue.front();
	CommandsDue.pop_front();
	this->works_completed++;
	this->mut.unlock();
	return cmd;
}

void CWorkScheduler::ScheduleWork(ICommand *Command) {
    // assert if either it as broadcast event with target!=NULL or if not an
    // broadcast event with the target equal NULL.
    assert(
        (Command->getCmd() > CMD_BROADCAST_MAX && NULL != Command->getTrgt()) ||
        (Command->getCmd() <= CMD_BROADCAST_MAX && 0 == Command->getTrgt()));

    this->mut.lock();
    CommandsDue.push_back(Command);
    this->works_received++;
    this->mut.unlock();
    sem_post(&semaphore);
}

void CWorkScheduler::ScheduleWork(ICommand *Command, struct timespec ts) {
    // assert if either it as broadcast event with target!=NULL or if not an
    // broadcast event with the target equal NULL.
    assert(
        (Command->getCmd() > CMD_BROADCAST_MAX && NULL != Command->getTrgt()) ||
        (Command->getCmd() <= CMD_BROADCAST_MAX && 0 == Command->getTrgt()));

    works_timed_scheduled++;
    timedwork->ScheduleWork(Command, ts);
}

