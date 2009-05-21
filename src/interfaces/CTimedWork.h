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

#include <cc++/thread.h>
#include "patterns/ICommand.h"
#include "CWorkScheduler.h"


/** This class bundles a timed activity.
 *
 * Currently, activities are tasks, which when ending, will enque an immediatte action.
 */
class CTimedWork : public  ost::Thread
{
public:
	/** Constructor: Takes the scheduler to inform, the command to execute and the time when.
	 */
	CTimedWork(CWorkScheduler *sch, ICommand *cmd, struct timespec ts);


private:
	CTimedWork() {};
	/** holds the commmand */
	ICommand *cmd;
	/** holds the timespec */
	struct timespec ts;
	/** it is attached to this scheduler.
	 * (The scheduler keeps books of its processes)*/
	CWorkScheduler *sch;

protected:
	/// Called on thread termination (See commonc++ lib)
	void final();

	/// Called on execution of the thread. (See commonc++ lib)
	void run();
};

#endif /* CTIMEDWORK_H_ */
