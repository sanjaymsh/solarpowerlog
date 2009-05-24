/* ----------------------------------------------------------------------------
   solarpowerlog
   Copyright (C) 2009  Tobias Frost

   This file is part of solarpowerlog.

   Solarpowerlog is free software; However, it is dual-licenced
   as described in the file "COPYING".

   For this file (ICommandTarget.h), the license terms are:

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


/** \file ICommandTarget.h
 *
 *  Created on: May 17, 2009
 *      Author: tobi
 */

#ifndef ICOMMANDTARGET_H_
#define ICOMMANDTARGET_H_

#include <iostream>

#include <assert.h>

#include <time.h>

#include "ICommand.h"

/** Interface for a command's target.
 *
 *
 * TODO DOCUMENT ME!
 */
class ICommandTarget {
public:
	ICommandTarget();
	virtual ~ICommandTarget();

	virtual void ExecuteCommand(const ICommand *Command) = 0 ;
};

#endif /* COMMANDTARGET_H_ */
