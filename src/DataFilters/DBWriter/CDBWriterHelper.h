/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2009-2014 Tobias Frost

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
 * CDBWriterHelper.hpp
 *
 *  Created on: 05.07.2014
 *      Author: tobi
 */

#ifndef DDBWRITER_HELPER_HPP_
#define DDBWRITER_HELPER_HPP_

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef  HAVE_FILTER_DBWRITER

#include <string>
#include "configuration/ILogger.h"
#include "patterns/IObserverObserver.h"
#include "Inverters/interfaces/InverterBase.h"

/** cache the information in the config what and where we should log to.
 *
 * Brainstorming;
 * - input capabilites / column names
 * - table names
 * - operational mode
 * - observer pattern support (note which capabilities are already subscribed,
 *   )
 *
 *
 * */
class CDBWriterHelper : public IObserverObserver
{

public:

    CDBWriterHelper(IInverterBase *base, const ILogger &parent, const std::string &table,
        const std::string &mode, bool logchangedonly, float logevery);

    /// Add the tuple Capability, Column to the "should be logged information"
    /// Returns "FALSE" if the combination of Capabilty and Column is alreaedy there.
    bool AddDataToLog(const std::string &Capability, const std::string &Column);

    virtual void Update(const class IObserverSubject * subject);

    float _logevery;
    bool _logchangedonly;

private:

    bool issane(const std::string s);
    enum omode
    {
        continuous, single, cumulative
    };

    class Cdbinfo
    {
    public:
    Cdbinfo(std::string Capability, std::string Column, bool wasUpdated = false,
        bool wasSeen = false) :
        Capability(Capability), Column(Column), wasUpdated(wasUpdated),
            wasSeen(wasSeen), previously_subscribed(false)
    {};

    std::string Capability;
    std::string Column;
    bool wasUpdated;
    bool wasSeen;
    bool previously_subscribed; // just to supress a debug message.
    };

    ILogger logger;

    std::vector<Cdbinfo> _dbinfo;

    std::string _table;
    omode _mode;

    bool _table_sanizited;
    bool _datavalid;

    // The DB-Writer's parent
    IInverterBase *_base;

};

#endif

#endif /* DDBWRITER_HELPDER_HPP_ */
