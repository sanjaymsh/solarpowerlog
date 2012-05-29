/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

 Copyright (C) 2009-2012 Tobias Frost

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ----------------------------------------------------------------------------
 */

/*
 * \file ISputnikCommandBackoffStrategy.cpp
 *
 *  Created on: 28.05.2012
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ISputnikCommandBackoffStrategy.h"

    /// Should the command be considered?
bool ISputnikCommandBackoffStrategy::ConsiderCommand()
{
    bool ret = true;
    if (next)
        ret = next->ConsiderCommand();
    return ret;
}

/// The command has been issued
/// Note: If "Command Issued" is followed by "Command Answered"
void ISputnikCommandBackoffStrategy::CommandIssued()
{
    if (next)
        next->CommandIssued();
}

/// The command has been answered.
void ISputnikCommandBackoffStrategy::CommandAnswered()
{
    if (next)
        next->CommandAnswered();
}

/// Inverter disconnected.
void ISputnikCommandBackoffStrategy::Reset()
{
    if (next)
        next->Reset();
}
