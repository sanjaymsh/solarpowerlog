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
 * CSputnikCommandSoftwareVersion.cpp
 *
 *  Created on: 19.05.2012
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#include "CSputnikCommandSoftwareVersion.h"
#include "configuration/ILogger.h"

/// command string for Software Version
static const std::string SWV("SWV");
/// command string for BuildVersion
static const std::string BDN("BDN");


int CSputnikCommandSoftwareVersion::GetMaxAnswerLen(void) {
    if ( ! this->got_swversion ) return 9;
    return 7;
}

const std::string& CSputnikCommandSoftwareVersion::GetCommand(void) {
    if ( !got_swversion) return SWV;
    return BDN;
}

bool CSputnikCommandSoftwareVersion::IsHandled(const std::string& token) {
    if ( token == SWV) return true;
    if ( token == BDN) return true;
    return false;
}

bool CSputnikCommandSoftwareVersion::ConsiderCommand() {
    if (!this->got_buildversion || !this->got_swversion) return true;
#warning implement backoff algorithm!
    // temporary: fixed backoff. Later, a algorithm should consider when
    // its time to do the next command.
    // In this case, only once per connection.
    if (backoff-- > 0) return false;
    got_buildversion = false;
    got_swversion = false;
    backoff = 10;
    return true;
}

void CSputnikCommandSoftwareVersion::handle_token(
    const std::vector<std::string>& tokens) {

    if ( tokens[0] == SWV ) {
        sw = strtoul(tokens[1].c_str(), NULL, 16);
        got_swversion = true;
    }
    else if ( tokens[0] == BDN ) {
        build = strtoul(tokens[1].c_str(), NULL, 16);
        got_buildversion = true;
    }
    else
    {
        LOGDEBUG(inverter->logger,
            "CSputnikCommandSoftwareVersion::handle_token() unexpected call "
                << tokens[0]);
        return;
    }

    std::string strsw;
    strsw = sw/10 + "." + sw%10;
    if ( got_buildversion ) {
        strsw += "Build " + build;
    }

    this->CapabilityHandling<std::string>(strsw);
}
