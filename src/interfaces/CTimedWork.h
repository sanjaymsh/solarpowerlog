/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CTimedWork.h), the license terms are:

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

/** \file CTimedWork.h
 *
 *  Created on: May 18, 2009
 *      Author: tobi
 */

#ifndef CTIMEDWORK_H_
#define CTIMEDWORK_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "patterns/ICommand.h"
#include "CWorkScheduler.h"
#include "boost/date_time/posix_time/posix_time.hpp"

#include <boost/thread.hpp>

/** This class bundles a timed activity.
 *
 * Currently, activities are tasks, which when ending, will enque an immediatte action.
 */
class CTimedWork
{
public:
	/** Constructor: Takes the scheduler to inform, the command to execute and the time when.
	 */
	explicit CTimedWork( CWorkScheduler *sch );

	virtual ~CTimedWork();

	/// Thread entry point.
	void run();

	void ScheduleWork( ICommand *Command, struct timespec ts );

	/// Ask thread to terminate.
	void RequestTermination( void );

private:
	CTimedWork()
	{
	}

	void _main( void );

#if 1
	struct time_compare
	{
		bool operator()( const boost::posix_time::ptime t1,
			const boost::posix_time::ptime t2 ) const
		{
			if (t1 > t2)
				return false;
			return true;
		}
		;
	};

	multimap<boost::posix_time::ptime, ICommand*, time_compare>
		TimedCommands;
#endif
	/** holds the commmand */
	ICommand *cmd;
	/** holds the timespec */
	struct timespec ts;
	/** it is attached to this scheduler.
	 * (The scheduler keeps books of its processes)*/
	CWorkScheduler *sch;

	volatile bool terminate;

	boost::thread thread;

	boost::mutex mut;

};

#endif /* CTIMEDWORK_H_ */
