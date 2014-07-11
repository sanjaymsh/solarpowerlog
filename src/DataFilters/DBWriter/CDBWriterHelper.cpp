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
 * CDBWriterHelper.cpp
 *
 *  Created on: 05.07.2014
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef  HAVE_FILTER_DBWRITER

#include "CDBWriterHelper.h"

#include "Inverters/Capabilites.h"
#include "Inverters/interfaces/ICapaIterator.h"
#include "patterns/CValue.h"
#include "interfaces/CMutexHelper.h"
#include "CDBWHSpecialTokens.h"


#include <assert.h>

CDBWriterHelper::CDBWriterHelper(IInverterBase *base, const ILogger &parent,
    const std::string &table, const std::string &mode,
    const std::string &createmode, bool logchangedonly,
    float logevery, bool allow_sparse)
{
    logger.Setup(parent.getLoggername(), "CDBWriterHelper(" + table + ")");
    _table = table;
    _table_sanizited = false;
    _logevery = logevery;
    _logchangedonly = logchangedonly;
    _base = base;
    _datavalid = false;
    _createtable_mode = CDBWriterHelper::cmode_no;
    _allow_sparse = allow_sparse;

    if (createmode == "YES") {
        _createtable_mode = CDBWriterHelper::cmode_yes;
    } else if (createmode == "YES-WIPE-MY-DATA") {
        _createtable_mode = CDBWriterHelper::cmode_yes_and_drop;
    }

    _olddatastate = NULL;

    if (mode == "continuous") {
        _mode = CDBWriterHelper::continuous;
    } else if (mode == "single") {
        _mode = CDBWriterHelper::single;
    } else if (mode == "cumulative") {
        _mode = CDBWriterHelper::cumulative;
    }

    assert(base);
    CCapability *cap;
    cap = base -> GetConcreteCapability(CAPA_CAPAS_UPDATED);
    assert(cap);
    cap->Subscribe(this);

    cap = base->GetConcreteCapability(CAPA_CAPAS_REMOVEALL);
    assert(cap);
    cap->Subscribe(this);

    cap = base->GetConcreteCapability(CAPA_INVERTER_DATASTATE);
    assert(cap);
    cap->Subscribe(this);
}

CDBWriterHelper::~CDBWriterHelper() {
#warning missing destructor
}

/** Add the tuple Capability, Column to the "should be logged information"
 * Returns "FALSE" if the combination of Capabilty and Column is alreaedy there.
*/
bool CDBWriterHelper::AddDataToLog(const std::string &Capability,
    const std::string &Column)
{

    assert(!Capability.empty());
    assert(!Column.empty());

    // Check if we need to complain on the tablename...
    if (!_table_sanizited && !issane(_table)) {
        LOGFATAL(logger, "Tablename is not sane: " << _table);
        return false;
    }
    _table_sanizited = true;

    if (!issane(Column)) {
        LOGFATAL(logger, "Columname is not sane: " << Column);
        return false;
    }

    std::vector<Cdbinfo>::iterator it;
    for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
        if ((*it).Column == Column) {
            LOGFATAL(logger,
                "Table " << _table << ": Column " << Column <<" already used as target:");
            return false;
        }
    }

    LOGDEBUG(logger,
        "\"" << Capability << "\" will be logged to column \"" << Column << "\"");

    _dbinfo.push_back(Cdbinfo(Capability, Column));

    Cdbinfo &last = _dbinfo[_dbinfo.size()-1];

    if (Capability[0] == '%') {
        LOGDEBUG(logger, "Special token " << Capability);
        IDBHSpecialToken *nt = CDBHSpecialTokenFactory::Factory(Capability);
        if (!nt) {
            LOGFATAL(logger,
                "Cannot create special token object " << Capability);
            return false;
        }

        time_t t = time(0);
        struct tm *tm = localtime(&t);
        nt->Update(*tm);
        last.Value = (IValue*)nt;
        last.LastLoggedValue = NULL;
        last.isSpecial = true;

        // Debug code to test the IDBSpecialToken -> IValue interface.
        //time_t t = time(0);
        //struct tm *tm = localtime(&t);
        //nt->Update(*tm);
        //LOGDEBUG(logger, nt->GetString());
        //IDBHSpecialToken *tst = (IDBHSpecialToken*) last.Value;
        //tst->Update(*tm);
        //LOGDEBUG(logger, tst->GetString());
    }
    return true;
}

bool CDBWriterHelper::issane(const std::string s)
{
    // needle is selected from mysqli.real-escape-string
    // I added the brackets, which might be overkill...
    const char *needle = "\"\'`[]\0\r\n\x1a%";
    if (std::string::npos != s.find_first_of(needle, 0, sizeof(needle))) {
        return false;
    }
    return true;
}

void CDBWriterHelper::Update(const class IObserverSubject * subject)
{
    assert(subject);
    CCapability *cap = (CCapability *)subject;

    // Datastate changed.
    if (cap->getDescription() == CAPA_INVERTER_DATASTATE) {
        bool current = _datavalid;
        _datavalid = ((CValue<bool> *)cap->getValue())->Get();
        if (current != _datavalid && !_datavalid) {
            // data just became invalid.
            // reset our internal states.
            std::vector<Cdbinfo>::iterator it;
            CMutexAutoLock cma(&mutex);
            for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
                Cdbinfo &cit = (*it);
                if (cit.Value && !cit.isSpecial) {
                    delete cit.Value;
                    cit.Value = NULL;
                }
                if (cit.LastLoggedValue && !cit.isSpecial) {
                    delete cit.LastLoggedValue;
                    cit.LastLoggedValue = NULL;
                }
                cit.wasUpdated = false;
            }
        }
#if 0
        // debug code I want to keep for the moment
        // debugs the CValue == and != operator, at least for bool
        if ( ! _olddatastate )
        {
            _olddatastate = cap->getValue()->clone();
        }
        else {
            IValue &o = *cap->getValue();
            IValue &n = *_olddatastate;
            LOGDEBUG(logger, "OLD=" << ((CValue<bool> &)o).Get() <<
                " NEW=" << ((CValue<bool> &)n).Get());
            if ( o == n ) LOGDEBUG(logger, "EQUAL");
            if ( o != n ) LOGDEBUG(logger, "NOT EQUAL");
        }
#endif

        return;
    }

    // Unsubscribe plea -- we do not offer this Capa, our customers will
    // ask our base directly.
    if (cap->getDescription() == CAPA_CAPAS_REMOVEALL) {
        auto_ptr<ICapaIterator> it(_base->GetCapaNewIterator());
        while (it->HasNext()) {
            pair<string, CCapability*> cappair = it->GetNext();
            cap = (cappair).second;
            cap->UnSubscribe(this);
        }
        return;
    }

    //  "caps updated" -- there might be new capabilities...
    // Iterate through all capabilities and if we have customers, subscribe to it

    if (cap->getDescription() == CAPA_CAPAS_UPDATED) {
        auto_ptr<ICapaIterator> it(_base->GetCapaNewIterator());
        while (it->HasNext()) {
            std::string capname;
            std::vector<Cdbinfo>::iterator jt;
            pair<string, CCapability*> cappair = it->GetNext();
            cap = (cappair).second;
            capname = cap->getDescription();
            for (jt = _dbinfo.begin(); jt != _dbinfo.end(); jt++) {
                if ((*jt).Capability == capname) {
                    CMutexAutoLock cma(&mutex);
                    if (!(*jt).previously_subscribed)
                        LOGDEBUG(logger, "Subscribing to " << cap->getDescription());
                    cap->Subscribe(this);
                    (*jt).previously_subscribed = true;
                    break;
                }
            }
        }
        return;
    }

    // OK, some caps has been updated. Lets clone the value :)
    std::vector<Cdbinfo>::iterator it;
    std::string capaname = cap->getDescription();

    for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
        if ((*it).Capability == capaname) {
            CMutexAutoLock cma(&mutex);
            Cdbinfo &cit = *it;
            // Check if the value changed since last logging to the DB
            if (cit.LastLoggedValue) {
                IValue &last = *cit.LastLoggedValue;
                IValue &now  = *cap->getValue();
                if ( now != last) cit.wasUpdated = true;
            } else {
                cit.wasUpdated = true;
            }

            // Check if this is the first time we've got the value.
            if (cit.Value) delete cit.Value;
            cit.Value = cap->getValue()->clone();
            break;
        }
    }
}

bool CDBWriterHelper::ExecuteQuery(cppdb::session &session) {


    // Strategie
    // only log if data is marked as valid
    //
    // For "continious" mode just add new columns
    // "sparse mode" -> add NULL if no data is there (log even if all_available is false)

    // If creating table, "sparse mode" cannot be used -- datatypes are needed to assemble the create statement.

    // In "single mode" the statement will need to have a selector to update the right row (smth like WHERE column == zzz)
    // This selektors are defined by a leading "$" in the Capability and the remaining part will be (after checking if it
    // is "sane") added to the satement. The column specifies the column, you guessed right :).

    // "Cumulative" mode first needs to try to update the row in question, and if that fails, try to add the row.

    // paranoid safety checks
    if (!_table_sanizited) return false;

    // Nothing to do...
    if (!_datavalid) return true;

    // check what we've got so far
    std::vector<Cdbinfo>::iterator it;
    bool any_updated = false;
    bool all_available = true;
    bool special_updated = true;

    time_t t = time(0);
    struct tm *tm = localtime(&t);

    for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
        CMutexAutoLock cma(&mutex);
        Cdbinfo &cit = *it;
        if (!cit.Value) all_available = false;
        if (cit.wasUpdated) any_updated = true;
        if (cit.isSpecial) {
            IDBHSpecialToken *st = (IDBHSpecialToken*) cit.Value;
            if (st->Update(*tm)) special_updated=true;
            LOGDEBUG(logger, "Updated " << cit.Capability << "value " << st->GetString());
        }
    }

    // Part 1 -- create table if necessary.
    if (_createtable_mode != CDBWriterHelper::cmode_no) {
        // Creation of the table is only possible if we have all data, as we need to know the
        // datatyptes
        if (!all_available) {
            LOGDEBUG(logger, "Not all data available to create table " << _table);
            return true;
        }

        // CREATE TABLE table ( created TIMESTAMP,
        // month int, day int, hour int

        // Iterate through all specs and assemble table.
        for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
            Cdbinfo &info = *it;
            if (info.Capability[0] == '$') {

            }

        }


#warning todo
    }


    // Part 2 -- create sql statement for adding / replacing ... data.




}


#endif
