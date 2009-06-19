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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <list>
#include <time.h>

#include "CTimedWork.h"

#include "interfaces/CMutexHelper.h"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "interfaces/CMutexHelper.h"

CTimedWork::CTimedWork( CWorkScheduler *sch )
{
	this->sch = sch;
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
		CMutexAutoLock lock(&mut);
		TimedCommands.insert(
			pair<boost::posix_time::ptime, ICommand*> (n, Command));
	}

	thread.interrupt();
}

void CTimedWork::_main()
{
	boost::posix_time::ptime n, w;
	boost::posix_time::time_duration s;

	while (!terminate) {
		// get now
		n = boost::posix_time::microsec_clock::local_time();

#if 0
		cerr << "Now: " << n << endl;
#endif

		mut.lock();

#if 0
		cerr << "TIMED WORKS EXPIRATION" << endl;
		multimap<boost::posix_time::ptime, ICommand*>::iterator it;
		for (it = TimedCommands.begin(); it != TimedCommands.end(); it++) {
			cerr << (*it).first << endl;
		}
		cerr << "TIMED WORKS LIST END" << endl;
#endif

		if (!TimedCommands.empty()) {
			w = (TimedCommands.begin())->first;
#if 0
			cerr << "Waiting: " << to_simple_string(w) << endl;
#endif
			if (w > n) {
				s = w - n;
#if 0
				cerr << "Difference: " << to_simple_string(s)
					<< endl;
#endif
			} else {
				ICommand *cmd = (TimedCommands.begin())->second;
				TimedCommands.erase(TimedCommands.begin());
				mut.unlock();
				sch->ScheduleWork(cmd);
				continue;
			}
		} else {
			s = boost::posix_time::hours(1);
		}

		mut.unlock();

		try {
#if 0
			cerr << "sleeping for "
				<< boost::posix_time::to_simple_string(s)
				<< endl;
#endif
			boost::this_thread::sleep(s);
		} catch (boost::thread_interrupted e) {
#if 0
			cerr << " Sleep interrupted " << endl;
#endif
		}
	}
}

void CTimedWork::RequestTermination( void )
{
	terminate = true;
	thread.interrupt();
}

