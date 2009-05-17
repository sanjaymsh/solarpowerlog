/* ----------------------------------------------------------------------------
   solarpowerlog
   Copyright (C) 2009  Tobias Frost

   This file is part of solarpowerlog.

   Solarpowerlog is free software; However, it is dual-licenced
   as described in the file "COPYING".

   For this file (ICommand.cpp), the license terms are:

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

