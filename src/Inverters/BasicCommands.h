/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2009-2012 Tobias Frost

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ----------------------------------------------------------------------------
 */

#ifndef _BASIC_COMMANDS_H
#define _BASIC_COMMANDS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

enum BasicCommands {

	/// reserverd for transport algorithms which can send events
	/// on receiption (planned... therefore TODO )
	CMD_RECEIVED ,
	// For now, we reserve 1000 cmds for our purpose.
	// but please use this define for your commands.
	CMD_USER = 1000
};


#endif
