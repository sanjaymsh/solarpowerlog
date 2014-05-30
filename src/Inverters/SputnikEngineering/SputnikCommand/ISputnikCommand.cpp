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
 * ISputnikCommand.cpp
 *
 *  Created on: 19.05.2012
 *      Author: tobi
 */

#include "ISputnikCommand.h"
#include "Inverters/SputnikEngineering/SputnikCommand/BackoffStrategies/CSputnikCmdBOAlways.h"

ISputnikCommand::ISputnikCommand( ILogger &logger, const std::string &command,
        int max_answer_len, IInverterBase *inv, const std::string & capaname,
        ISputnikCommandBackoffStrategy *backoffstrategy ) :
        command(command), max_answer_len(max_answer_len), inverter(inv), capaname(
                capaname), logger(logger)
{
    if (backoffstrategy) {
        this->strat = backoffstrategy;
    } else {
        this->strat = new CSputnikCmdBOAlways(NULL);
    }

#ifdef DEBUG_BACKOFFSTRATEGIES
    this->strat->SetLogger(logger);
#endif
}

ISputnikCommand::~ISputnikCommand()
{
    delete strat;
}

bool ISputnikCommand::ConsiderCommand()
{
    return strat->ConsiderCommand();
}

const std::string & ISputnikCommand::GetCommand( void )
{
    return command;
}

unsigned int ISputnikCommand::GetCommandLen(void) {
    return command.length();
}

bool ISputnikCommand::IsHandled(const std::string &token) {
     return (token == command);
 }
