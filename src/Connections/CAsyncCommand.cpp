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
	if (private_icommand)
		delete callback;
}

