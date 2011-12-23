/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CAsyncCommand.cpp), the license terms are:

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

/*
 * CAsyncCommand.cpp
 *
 *  Created on: Dec 14, 2009
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#include "configuration/Registry.h"
#include "interfaces/CWorkScheduler.h"
#include "Connections/CAsyncCommand.h"

CAsyncCommand::CAsyncCommand(enum Commando c, ICommand *callback, sem_t *sem)
{
	this->c = c;
	if (!callback) {
		this->callback = new ICommand(NULL, NULL);
		private_icommand = true;
	} else {
		this->callback = callback;
		private_icommand = false;
	}

	this->sem = sem;
}

CAsyncCommand::~CAsyncCommand()
{
	// if we created the callback, delete it.
	// else, we are not owner of it, and so we cannot delete it.
	if (private_icommand) {
		delete callback;
	}
}

void CAsyncCommand::HandleCompletion( void )
{
	if (!private_icommand) {
		Registry::GetMainScheduler()->ScheduleWork(callback);
	} else {
		sem_post(sem);
	}
}

