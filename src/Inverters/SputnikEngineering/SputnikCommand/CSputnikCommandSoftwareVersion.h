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
 * CSputnikCommandSoftwareVersion.h
 *
 *  Created on: 19.05.2012
 *      Author: tobi
 */

#ifndef CSPUTNIKCOMMANDSOFTWAREVERSION_H_
#define CSPUTNIKCOMMANDSOFTWAREVERSION_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ISputnikCommand.h"

/** Special implementation for the Inverter's Firmware Version property.
 *
 * To get that information, two commands are required. (SWV and BDN)
 *
 */
class CSputnikCommandSoftwareVersion : public ISputnikCommand
{
    CSputnikCommandSoftwareVersion(const std::string &cmd,
        IInverterBase *inv, const std::string & capname)
        : ISputnikCommand(cmd, 0, inv, capname), got_buildversion(false), got_swversion(false), backoff(10) {
    }

    virtual bool ConsiderCommand();

    virtual int GetMaxAnswerLen(void);

    virtual const std::string& GetCommand(void);

    virtual bool IsHandled(const std::string &token);

    virtual void handle_token(const std::vector<std::string> & tokens);

private:
    bool got_buildversion;
    bool got_swversion;
    long sw,build;

    int backoff;

};

#endif
