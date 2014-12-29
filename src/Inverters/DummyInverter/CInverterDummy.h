/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009-2011  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licensed
 as described in the file "COPYING".

 For this file (CInverterDummy.h), the license terms are:

 You can redistribute it and/or  modify it under the terms of the GNU Lesser
 General Public License (LGPL) as published by the Free Software Foundation;
 either version 3 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
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

#ifdef HAVE_INV_DUMMY

#include "Inverters/interfaces/InverterBase.h"

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
		CMD_INIT
	};

};

#endif /* HAVE_INV_DUMMY */

#endif /* CINVERTERDUMMY_H_ */
