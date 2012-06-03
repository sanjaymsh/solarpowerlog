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

/**
 * \file ISputnikCommandBackoffStrategy.h
 *
 * The backoff strategies decided how often a command will be issued
 * due to some compile-time selected algorithms.
 *
 * Also (they will) determine if a command is at all supported and if
 * necessary will cease this command completely. (
 *
 * This class defines the interface for the algorithms.
 *
 * Note that Backoff-Stragetgies can be staggered (similar to the
 * decorator Pattern), therefore derived classes needs always to call
 * the interface method.
 *
 *  Created on: 28.05.2012
 *      Author: tobi
 */

#ifndef ISPUTNIKCOMMANDBACKOFFSTRATEGY_H_
#define ISPUTNIKCOMMANDBACKOFFSTRATEGY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stddef.h>

class ISputnikCommandBackoffStrategy
{
public:

    ISputnikCommandBackoffStrategy(ISputnikCommandBackoffStrategy *next = NULL) {
        this->next = next;
    }

    virtual ~ISputnikCommandBackoffStrategy() {
        if (next) delete next;
    };

    /// Should the command be considered?
    /// return false if not, true if yes.
    /// Call Interface first. If Interface returns "false", also return false.
    virtual bool ConsiderCommand();

    /// The command has been issued
    /// Note: If "Command Issued" is followed by "Command Answered" or
    /// a "CommandNotAnswered".
    virtual void CommandIssued() ;

    /// The command has been answered.
    virtual void CommandAnswered();

    /// The command has not been answered.
    virtual void CommandNotAnswered();

    /// Inverter disconnected, reset state.
    virtual void Reset();

protected:
    ISputnikCommandBackoffStrategy *next;
};

#endif /* ISPUTNIKCOMMANDBACKOFFSTRATEGY_H_ */
