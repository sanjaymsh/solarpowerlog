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
#include <cppdb/frontend.h>

#include "configuration/ILogger.h"
#include "patterns/IObserverObserver.h"
#include "Inverters/interfaces/InverterBase.h"

#include "CdbInfo.h"

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

    CDBWriterHelper(IInverterBase *base, const ILogger &parent,
        const std::string &table, const std::string &mode,
        const std::string &createmode, bool logchangedonly, float logevery,
        bool allow_sparse);

    virtual ~CDBWriterHelper();

    // The SQL Magic....
    virtual void ExecuteQuery(cppdb::session &session);

    /// Add the tuple Capability, Column to the "should be logged information"
    /// Returns "FALSE" if the combination of Capabilty and Column is alreaedy there.
    bool AddDataToLog(const std::string &Capability, const std::string &Column);

    virtual void Update(const class IObserverSubject * subject);

    const std::string &GetTable(void) {
        return _table;
    }

    float _logevery;
    bool _logchangedonly;


private:

    bool issane(const std::string s);

    enum omode
    {
        continuous, single, cumulative
    };

    enum cmode
    {
        cmode_no, cmode_yes, cmode_yes_and_drop, cmode_print_statment
    };

    ILogger logger;


    std::string _table;
    omode _mode;

    bool _table_sanizited;
    bool _datavalid;

    bool _allow_sparse;

    /// The DB-Writer's parent
    IInverterBase *_base;

    IValue *_olddatastate;

    cmode _createtable_mode;

    boost::mutex mutex;

    /// Storage for the individual data sets to be stored (one per column)
    std::vector<class Cdbinfo*> _dbinfo;

    /// Cache for "regular" insert sql statements -- we don't need to recalculate
    /// them all over
    /// (TODO check if we should also cache the cppdb::statement object)
    std::string _insert_cache;

    /// If an query failed, we need to temporary disable the "anything changed?"
    /// logic. If this is true, an error has happened, so we retry in every case.
    bool _laststatementfailed;


    // Helper functions

    /// Assemble a value-string from the dbinfos
    ///
    /// the generated form is:
    /// [col1]=?, [col2]=?, ... [colx]=?
    /// The "?" are for the values to be bound
    /// \returns the generated value string
    std::string _GetValStringForUpdate(void);

    /// Assemble a columnvalue-string from the dbinfos
     ///
     /// the generated form is:
     /// (col1,col2,col3) VALUES (?,?,?)
     /// The "?" are for the values to be bound
     /// \returns the generated value string
     std::string _GetValStringForInsert(bool with_selector=false);

    /// Bind all "?" in the (previously calculated) value string
    bool _BindValues(cppdb::statement &s, bool with_selector=false);

    bool _BindSingleValue(cppdb::statement &stat, Cdbinfo &info);

};

#endif

#endif /* DDBWRITER_HELPDER_HPP_ */
