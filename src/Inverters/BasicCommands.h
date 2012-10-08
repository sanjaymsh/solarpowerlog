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

namespace BasicCommands
{
enum BasicCommands
{

    // The events 0 - CMD_BROADCAST_MAX are reserved for eg. broadcast events.
    // Any event in this range will be distributed to any inverter and any
    // datafilter using the main CWorkScheduler (##Registry::GetMainScheduler())
    // and are registered to receive broadcast events.

    /// SIG_TERM has been received and we are asked to terminate as soon as possible
    /// (For example, generally get read to close down, flush and close files,
    /// abort I/O if possible...)
    /// \note that you might receive other events until the programm really terminates
    /// (for example, aborting I/Os might generate events)
    /// \note after receiving this event, "timed work"
    /// (submitted via \sa CWorkSchedule::ScheduleWork())
    /// will be accepted but never handled. However, immediately due events
    /// added after receiption of this event will still be handled.
    CMD_BRC_SHUTDOWN,
    CMD_BROADCAST_MAX,

    // The events between CMD_BROADCAST_MAX and CMD_USER_MIN are reserved
    // at this moment.
    CMD_USER_MIN = 1000
};

}

#endif
