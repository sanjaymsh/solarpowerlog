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

 This program is distributed in the hope that it will be useful, but
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <list>
#include <time.h>

#include "CTimedWork.h"

#include "interfaces/CMutexHelper.h"

#include <boost/thread.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "interfaces/CMutexHelper.h"

#include "configuration/Registry.h"

CTimedWork::CTimedWork( CWorkScheduler *sch ) : sch(sch), terminate(false), dhc("CTimedWork")
{
    this->work_completed=0; this->work_received=0;
    zero_waits = 0;
    dhc.Register(new CDebugObject<int>("work_received",work_received));
    dhc.Register(new CDebugObject<int>("work_completed",work_completed));

    thread_interrupts_count = thread_interrupts_sighandler = thread_interrupts_balance = 0;
    dhc.Register(new CDebugObject<int>("thread_interrupts_count",thread_interrupts_count));
    dhc.Register(new CDebugObject<int>("thread_interrupts_sighandler",thread_interrupts_sighandler));
    dhc.Register(new CDebugObject<int>("thread_interrupts_balance",thread_interrupts_balance));
    dhc.Register(new CDebugObject<int>("works_pending",works_pending));
    dhc.Register(new CDebugObject<int>("zero_waits",zero_waits));

    thread_at_wait_point = 0;
    thread_wants_mutex = 0;
    thread_has_mutex = 0;
    ctimedwork_has_mutex = 0;
    ctimedwork_wants_mutex = 0;

    dhc.Register(new CDebugObject<int>("thread_at_wait_point",thread_at_wait_point));
    dhc.Register(new CDebugObject<int>("thread_wants_mutex",thread_wants_mutex));
    dhc.Register(new CDebugObject<int>("ctimedwork_wants_mutex",ctimedwork_wants_mutex));
    dhc.Register(new CDebugObject<int>("thread_has_mutex",thread_wants_mutex));
    dhc.Register(new CDebugObject<int>("ctimedwork_has_mutex",ctimedwork_wants_mutex));

}

CTimedWork::~CTimedWork()
{
	if (!terminate)
		RequestTermination();
	thread.join();
}

// Called on execution of the thread.
void CTimedWork::run()
{
	thread = boost::thread(boost::bind(&CTimedWork::_main, this));
}

void CTimedWork::ScheduleWork( ICommand *Command, struct timespec ts )
{
	boost::posix_time::ptime n =
		boost::posix_time::microsec_clock::local_time();
	boost::posix_time::seconds s(ts.tv_sec);
	boost::posix_time::millisec ms(ts.tv_nsec / (1000*1000));
	n = n + s + ms;
	{
	    ctimedwork_wants_mutex=1;
		CMutexAutoLock m(&this->mut);
		ctimedwork_wants_mutex=1;ctimedwork_has_mutex=1;
		this->work_received++;

		TimedCommands.insert(
			pair<boost::posix_time::ptime, ICommand*> (n, Command));

		ctimedwork_wants_mutex=0;
		m.unlock(); // should not be needed, but paranoid me...
		ctimedwork_has_mutex=0;
	}

	(volatile int)thread_interrupts_balance++;
	thread_interrupts_count++;
	thread.interrupt();
	(volatile int)thread_interrupts_balance--;
	// LOGDEBUG(Registry::GetMainLogger(),"CTimedWork scheduled, thread.interrupt returned");
}

void CTimedWork::_main()
{
    boost::posix_time::ptime n, w;
    boost::posix_time::time_duration s;

    while (!terminate) {

        this->thread_wants_mutex=1;
        mut.lock();
        this->thread_has_mutex=1;

        // get now
        n = boost::posix_time::microsec_clock::local_time();

#if 0
        cerr << "Now: " << (boost::posix_time::ptime)n << endl;
#endif
#if 0
        cerr << "TIMED WORKS EXPIRATION" << endl;
        multimap<boost::posix_time::ptime, ICommand*>::iterator it;
        for (it = TimedCommands.begin(); it != TimedCommands.end(); it++) {
            cerr << (*it).first << endl;
        }
        cerr << "TIMED WORKS LIST END" << endl;
#endif

        this->works_pending = TimedCommands.size();

        if (!TimedCommands.empty()) {
            w = (TimedCommands.begin())->first;
            // cerr << "Waiting: " << to_simple_string(w) << endl;

             if (w > n) {
                s = w - n;
               // cerr << "Difference: " << to_simple_string(s)  << endl;
            } else {
                ICommand *cmd = (TimedCommands.begin())->second;
                TimedCommands.erase(TimedCommands.begin());

                this->works_pending = TimedCommands.size();
                work_completed++;

                this->thread_wants_mutex=0;
                mut.unlock();
                this->thread_has_mutex=0;

                sch->ScheduleWork(cmd);
                continue;
            }
        } else {
            this->zero_waits++; // wait requests for empty queues
            s = boost::posix_time::hours(1);
        }

        this->thread_wants_mutex=0;
        mut.unlock();
        this->thread_has_mutex=0;

        try {
            // cerr << "sleeping for " << boost::posix_time::to_simple_string(s) << endl;
            this->thread_at_wait_point=1;
            boost::this_thread::sleep(s);
            this->thread_at_wait_point=0;
        } catch (boost::thread_interrupted &e) {
            // sleep was interrupted, probably by new work.
            thread_interrupts_sighandler++;
        }
    }
}

void CTimedWork::RequestTermination( void )
{
	terminate = true;
	thread.interrupt();
}

