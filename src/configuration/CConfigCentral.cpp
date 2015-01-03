/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

 Copyright (C) 2009-2015 Tobias Frost

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

/** \file CConfigCentral.cpp
 *
 *  Created on: 02.01.2015
 *      Author: tobi
 */

#include "CConfigCentral.h"

bool CConfigCentral::CheckConfig(ILogger& logger, const std::string &configpath)
{
    bool ret = true;
    CConfigHelper helper(configpath);
    std::list<boost::shared_ptr<IConfigCentralEntry> >::iterator it;
    for (it = l.begin(); it != l.end(); it++) {
        if (!(*it)->CheckAndUpdateConfig(logger,helper)) ret = false;
    }

    return ret;
}

