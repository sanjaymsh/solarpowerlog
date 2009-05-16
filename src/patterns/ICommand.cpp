/** \file ICommand.cpp
 *
 *  Created on: May 17, 2009
 *      Author: tobi
 */

#include "patterns/ICommand.h"


/** Just the constructor taking all elements... **/
ICommand::ICommand(int command, ICommandTarget *target, void *dat)
{
	cmd = command; trgt = target; data = dat;
}

/** Destructor, even for no need for destruction */
ICommand::~ICommand()
{
}

/** Delegate the command to the one that should do the work */
void ICommand::execute()
{
	trgt->ExecuteCommand(this);
}

/** Getter for the private cmd field (field is for Commandees use) */
int ICommand::getCmd() const
{
	return cmd;
}

/** Getter for the private cmd field (field is for Commandees use) */
void *ICommand::getData() const
{
	return data;
}

