/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2011-2012 Tobias Frost

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
 * CInverterDummy.h
 *
 *  Created on: 17.07.2011
 *      Author: tobi
 */

#ifndef CINVERTERDUMMY_H_
#define CINVERTERDUMMY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_INV_DUMMY

#include "Inverters/interfaces/InverterBase.h"
#include "Inverters/BasicCommands.h"

class CInverterDummy: public IInverterBase
{
public:
	CInverterDummy(const string &name, const string & configurationpath);

	virtual ~CInverterDummy();

	virtual bool CheckConfig() {
		return this->connection->CheckConfig();
	}

	virtual void ExecuteCommand(const ICommand *Command);

private:
	enum CMDs {
		CMD_INIT = BasicCommands::CMD_USER_MIN
	};

};

#endif /* HAVE_INV_DUMMY */

#endif /* CINVERTERDUMMY_H_ */
