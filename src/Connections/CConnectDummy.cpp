/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CConnectDummy.cpp), the license terms are:

 You can redistribute it and/or modify it under the terms of the GNU
 General Public License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/** \file CConnectDummy.cpp
 *
 *  Created on: May 22, 2009
 *      Author: tobi
 */

#include "CConnectDummy.h"
#include <iostream>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

using namespace std;

CConnectDummy::CConnectDummy(const string &configurationname)
: IConnect(configurationname)
{
}

CConnectDummy::~CConnectDummy() {
	// TODO Auto-generated destructor stub
}

bool CConnectDummy::CheckConfig(void)
{
	return false;
}


