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

/*
 * CSputnikCommandTYP.h
 *
 *  Created on: 26.05.2012
 *      Author: tobi
 */

#ifndef CSPUTNIKCOMMANDTYP_H_
#define CSPUTNIKCOMMANDTYP_H_

#include "ISputnikCommand.h"

class CSputnikCommandTYP : public ISputnikCommand
{
public:
    CSputnikCommandTYP(IInverterBase *inv)
    : ISputnikCommand("TYP", 9, inv, CAPA_INVERTER_MODEL) {}

    virtual ~CSputnikCommandTYP() {}

    virtual bool handle_token(const std::vector<std::string> & tokens);
};

#endif /* CSPUTNIKCOMMANDTYP_H_ */
