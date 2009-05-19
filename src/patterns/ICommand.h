/* ----------------------------------------------------------------------------
   solarpowerlog
   Copyright (C) 2009  Tobias Frost

   This file is part of solarpowerlog.

   Solarpowerlog is free software; However, it is dual-licenced
   as described in the file "COPYING".

   For this file (ICommand.h), the license terms are:

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


/** \file ICommand.h
 *
 *  Created on: May 17, 2009
 *      Author: tobi
 */

#ifndef ICOMMAND_H_
#define ICOMMAND_H_

// #include "patterns/ICommandTarget.h"

class ICommandTarget;

/** Encapsulates a command
 *
 * See the command pattern for details....
 *
 * Commands are used, for example, to schedule work,
 * to check for completion, etc...
 *
 * NOTE: The one that calls execute should delete the object afterwards!
 *
 */
class ICommand {
public:
	ICommand(int command, ICommandTarget *target, void *dat = 0);

	virtual ~ICommand();

	void execute();

    int getCmd() const;

    void *getData() const;

private:
	int cmd;
	ICommandTarget *trgt;
	void *data;


};

#endif /* ICOMMAND_H_ */
