/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2009-2014 Tobias Frost

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

/**
 * \file CDanfossCommand.cpp
 *
 *
 *  Plans are that the CSC will take over the logic in the Inverter code step by step.
 *  1- (done) CSC will know about the command token, expected telegram length, do the parsing and return the raw value
 *  2- (done) CSC will be able to "scale" values with an factor.
 *  2- CSC will keep track if a command is supported by the Inverter and cease issuing command if not.
 *  3- (Specialisations of CSCs might handle for some commands more often than others.)
 *  4- (done) CSC will take the Capabilities handling: creating the Capabilities, updating the values, notfiing subscribers

 *
 *  Created on: 30.05.2014
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_INV_DANFOSS

// currently empty.

#endif
