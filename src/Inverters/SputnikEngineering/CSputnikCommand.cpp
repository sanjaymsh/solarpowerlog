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
 * \file CSputnikCommand.cpp
 *
 *  Defines and handles a single commmand the Sputnik Inverter.
 *
 *  Plans are that the CSC will take over the logic in the Inverter code step by step.
 *  1- CSC will know about the command token, expected telegram length, do the parsing and return the raw value
 *  2- CSC will be able to "scale" values with an factor.
 *  2- CSC will keep track if a command is supported by the Inverter and cease issuing command if not.
 *  3- (Specialisations of CSCs might handle for some commands more often than others.)
 *  4- CSC will take the Capabilities handling: creating the Capabilities, updating the values, notfiing subscribers
 *
 *  Strategy:
 *  - CSCs are kept in a list attached to the Inverters Object
 *  - The CSCS are instanciated on construction time of the Inverter's object with all the information they need to operate.
 *  5- CSC will be registered by the Inverter at Instanciation time by key parameters-

 *
 *  Created on: 14.05.2012
 *      Author: tobi
 */

#include "CSputnikCommand.h"
#include "Inverters/interfaces/InverterBase.h"
//

IInverterBase *base;

void test() {
  //  CSputnikCommand<long,long> c("ABC",3,2);
    CSputnikCommand<float> d("ABC",3,2,base,"CDEF");

}
