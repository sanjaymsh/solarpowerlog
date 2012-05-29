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
 * due to some compile-time selected algorithgms.
 *
 * Also (they will) determine if a command is at all supported and if
 * necessary will cease this command completly. (and hinting the calling
 * inverter object to remove the command completly to free memory)
 *
 *  Created on: 28.05.2012
 *      Author: tobi
 */

#ifndef ISPUTNIKCOMMANDBACKOFFSTRATEGY_H_
#define ISPUTNIKCOMMANDBACKOFFSTRATEGY_H_

class ISputnikCommandBackoffStrategy
{
public:
    ISputnikCommandBackoffStrategy();
    virtual ~ISputnikCommandBackoffStrategy();
};

#endif /* ISPUTNIKCOMMANDBACKOFFSTRATEGY_H_ */
