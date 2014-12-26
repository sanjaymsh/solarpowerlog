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

#define SQL_ESCAPE_CHAR_OPEN ""
#define SQL_ESCAPE_CHAR_CLOSE ""

CDBWriterHelper::CDBWriterHelper(IInverterBase *base, const ILogger &parent,
    const std::string &table, const std::string &mode,
    const std::string &createmode, bool logchangedonly, float logevery,
    bool allow_sparse)
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
    _laststatementfailed = false;

    if (createmode == "YES") {
        _createtable_mode = CDBWriterHelper::cmode_yes;
    } else if (createmode == "YES-WIPE-MY-DATA") {
        _createtable_mode = CDBWriterHelper::cmode_yes_and_drop;
    } else if (createmode == "print-sql-statement") {
        _createtable_mode = CDBWriterHelper::cmode_print_statment;
    }

    _olddatastate = NULL;

    if (mode == "continuous") {
        _mode = CDBWriterHelper::continuous;
    } else if (mode == "single") {
        _mode = CDBWriterHelper::single;
    } else if (mode == "cumulative") {
        _mode = CDBWriterHelper::cumulative;
    }

    if (base) {
        CCapability *cap;
        cap = base->GetConcreteCapability(CAPA_CAPAS_UPDATED);
        assert(cap);
        cap->Subscribe(this);

        cap = base->GetConcreteCapability(CAPA_CAPAS_REMOVEALL);
        assert(cap);
        cap->Subscribe(this);

        cap = base->GetConcreteCapability(CAPA_INVERTER_DATASTATE);
        assert(cap);
        cap->Subscribe(this);
    }
}

CDBWriterHelper::~CDBWriterHelper()
{

    std::vector<Cdbinfo*>::iterator it;
    for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
        delete *it;
    }
    _dbinfo.clear();
}

/** Add the tuple Capability, Column to the "should be logged information"
 * Returns "FALSE" if the combination of Capabilty and Column is already there.
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

    std::vector<Cdbinfo*>::iterator it;
    for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
        if ((*it)->Column == Column) {
            LOGFATAL(logger,
                "Table " << _table << ": Column " << Column <<" already used as target:");
            return false;
        }
    }

    LOGDEBUG(logger,
        "\"" << Capability << "\" will be logged to column \"" << Column << "\"");

    class Cdbinfo* n = new Cdbinfo(Capability, Column);
    _dbinfo.push_back(n);

    Cdbinfo &last = *(_dbinfo[_dbinfo.size() - 1]);

    if (Capability[0] == '%' || Capability[0] == '!') {
        LOGDEBUG(logger, "Special token " << Capability);
        IDBHSpecialToken *nt = CDBHSpecialTokenFactory::Factory(
            Capability.substr(1));
        if (!nt) {
            LOGFATAL(logger,
                "Cannot create special token object " << Capability);
            return false;
        }

        time_t t = time(0);
        struct tm *tm = localtime(&t);
        nt->Update(*tm);
        last.Value = dynamic_cast<IValue*>(nt);
        assert(last.Value);
        last.LastLoggedValue = NULL;
        last.isSpecial = true;
    }

    if (Capability[0] == '$') {
        // Selektor.
        LOGDEBUG(logger, "Selector token " << Capability);
        if (Capability.length() <= 1) {
            LOGERROR(logger, "Selector token must not be empty.");
            return false;
        }
    }
    return true;
}

bool CDBWriterHelper::issane(const std::string s)
{
    // needle is selected from mysqli.real-escape-string
    // I added the brackets, which might be overkill...
    const char *needle = "\"\'`[]\0\r\n\x1a% ";
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
            std::vector<class Cdbinfo*>::iterator it;
            CMutexAutoLock cma(mutex);
            for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
                Cdbinfo &cit = **it;
                if (cit.Value && !cit.isSpecial) {
                    LOGTRACE(logger, "Update() deleting cit.Value for " << cit.Capability);
                    delete cit.Value;
                    cit.Value = NULL;
                }
                if (cit.LastLoggedValue && !cit.isSpecial) {
                    LOGTRACE(logger, "Update() deleting LastLoggedValue for " << cit.Capability);
                    delete cit.LastLoggedValue;
                    cit.LastLoggedValue = NULL;
                }
            }
        }
        return;
    }

    if (_datavalid) {
        LOGTRACE(logger,
            /*LOG_SA_HASH("update()-datastate"),*/
            "Update() Datastate valid");
    }
    else  {
        LOGTRACE(logger,
            /*LOG_SA_HASH("update()-datastate"),*/
            "Update() Datastate INvalid");
    }

    // Unsubscribe plea -- we do not offer this Capa, our customers will
    // ask our base directly.
    if (cap->getDescription() == CAPA_CAPAS_REMOVEALL) {
        LOGDEBUG(logger, "Update() CAPA_CAPAS_REMOVEALL received");
        auto_ptr<ICapaIterator> it(_base->GetCapaNewIterator());
        while (it->HasNext()) {
            pair<string, CCapability*> cappair = it->GetNext();
            cap = (cappair).second;
            LOGTRACE(logger, "Update() unsubscribing " << cap->getDescription());
            cap->UnSubscribe(this);
        }
        CMutexAutoLock cma(mutex);

        std::vector<class Cdbinfo *>::iterator jt;
        for (jt = _dbinfo.begin(); jt != _dbinfo.end(); jt++) {
            (*jt)->previously_subscribed = false;
            if ((*jt)->Value) delete ((*jt)->Value);
            ((*jt)->Value) = NULL;
        }
        return;
    }

    //  "caps updated" -- there might be new capabilities...
    // Iterate through all capabilities and if we have customers, subscribe to it

    if (cap->getDescription() == CAPA_CAPAS_UPDATED) {
        LOGDEBUG(logger, "Update() CAPA_CAPAS_UPDATED received");
        auto_ptr<ICapaIterator> it(_base->GetCapaNewIterator());
        while (it->HasNext()) {
            std::string capname;
            std::vector<class Cdbinfo *>::iterator jt;
            pair<string, CCapability*> cappair = it->GetNext();
            cap = (cappair).second;
            capname = cap->getDescription();
            for (jt = _dbinfo.begin(); jt != _dbinfo.end(); jt++) {
                if ((*jt)->Capability == capname) {
                    CMutexAutoLock cma(mutex);
                    if (!(*jt)->previously_subscribed)
                    LOGTRACE(logger,
                        "Update() Subscribing to " << cap->getDescription());
                    cap->Subscribe(this);
                    (*jt)->previously_subscribed = true;
                    break;
                }
            }
        }
        return;
    }

    // OK, some caps has been updated. Lets clone the value :)
    std::vector<class Cdbinfo*>::iterator it;
    std::string capaname = cap->getDescription();

    LOGTRACE(logger, capaname << " updated.");
    for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
        if ((*it)->Capability == capaname) {
            LOGTRACE(logger, "Update() found in Cdbinfo list");
            CMutexAutoLock cma(mutex);
            Cdbinfo &cit = **it;

            // Check if this is the first time we've got the value.
            LOGTRACE(logger, "Update() updating deleting cit.Value for " << cit.Capability);
            if (cit.Value) delete cit.Value;
            cit.Value = cap->getValue()->clone();
            break;
        }
    }
}

void CDBWriterHelper::ExecuteQuery(cppdb::session &session)
{
    // Strategy
    // only log if data is marked as valid
    //
    // For "continuous" mode just add new columns
    // "sparse mode" -> add NULL if no data is there (log even if all_available is false)

    // If creating table, "sparse mode" cannot be used -- datatypes are needed to assemble the create statement.

    // In "single mode" the statement will need to have a selector to update the right row (smth like WHERE column == zzz)
    // This selektors are defined by a leading "$" in the Capability and the remaining part will be (after checking if it
    // is "sane") added to the satement. The column specifies the column, you guessed right :).

    // "Cumulative" mode first needs to try to update the row in question, and if that fails, try to add the row.

    // paranoid safety checks
    if (!_table_sanizited) {
        LOGDEBUG(logger, "BUG: " << _table << " not sanitized");
        return;
    }

    // Nothing to do...
    if (!_datavalid) {
        LOGDEBUG_SA(logger,LOG_SA_HASH("ExecuteQuery_datavalid"),
            "ExecuteQuery() " <<_table << "Data invalid");
        return;
    }
    else {
        LOGDEBUG_SA(logger,LOG_SA_HASH("ExecuteQuery_datavalid"),
            "ExecuteQuery() " << _table << "Data valid");
    }

    // We need to lock the mutex to ensure data consistency
    // ( Precautionious -- solarpowerlog is currently only single task, if it
    // comes to processing; but there are ideas to put db handling in a thread,
    // and then it will be needed.)
    CMutexAutoLock cma(mutex);

    // check what we've got so far
    std::vector<class Cdbinfo*>::iterator it;

    // if lastsatementfailed is true, we have in any case do the work
    // (as this indicates that the last query failed.)
    bool any_updated = _laststatementfailed;
    bool all_available = true;
    bool special_updated = false;

    time_t t = time(0);
    struct tm *tm = localtime(&t);

    for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
        Cdbinfo &cit = **it;

        if (cit.Capability[0] == '$') {
            // don't consider selectors here.
            continue;
        }

        if (!cit.Value) {
            LOGDEBUG_SA(logger, LOG_SA_HASH("value-not-avail") + (long)(*it),
                "Value for " << cit.Capability << " is not available");
            all_available = false;
            continue;
        } else {
            LOGDEBUG_SA(logger, LOG_SA_HASH("value-not-avail") + (long)(*it),
                "Value for " << cit.Capability << " is available");
        }

        // previously logged?
        // if no (lastloggedvalue==0), we have a change
        // if lastloggedvalue and ivalue, we compare...
        if (cit.LastLoggedValue && cit.Value) {
            IValue &last = *cit.LastLoggedValue;
            IValue &now = *cit.Value;
            if (now != last) {
                any_updated = true;
                last = now;
            }
        } else if (cit.Value && !cit.isSpecial) {
            any_updated = true;
            cit.LastLoggedValue = cit.Value->clone();
        }

        if (cit.isSpecial) {
            IDBHSpecialToken *st = dynamic_cast<IDBHSpecialToken*>(cit.Value);
            if (st->Update(*tm)) {
                // only consider updated special values if they used as a selector.
                if (cit.Capability[0] == '!') special_updated = true;
            }
            LOGTRACE_SA(logger, LOG_SA_HASH("cit-updated") + (long)(*it),
                "Updated " << cit.Capability << " to value " << st->GetString());
        }
    }

    LOGTRACE_SA(logger, __COUNTER__, "Status: all_available=" << all_available
        << " any_updated=" << any_updated << " special_updated="
        << special_updated);

    // Part 1 -- create table if necessary.
    if (_createtable_mode != CDBWriterHelper::cmode_no) {
        LOGTRACE_SA(logger, __COUNTER__, "_createtable_mode != no");
        // Creation of the table is only possible if we have all data, as we need to know the
        // datatyptes
        if (!all_available) {
            LOGDEBUG_SA(logger, LOG_SA_HASH("not-all-data-there"),
                "Not all data available for CREATE TABLE " << _table << ". Retrying later.");
            return;
        } else {
            LOGDEBUG_SA(logger, LOG_SA_HASH("not-all-data-there"),
                "All data available for CREATE TABLE " << _table);
        }

        // Iterate through all specs and assemble table.
        std::string tablestring;
        for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
            Cdbinfo &info = **it;
            if (info.Capability[0] == '$') {
                // Special Select-Token we going to ignore
                LOGWARN_SA(logger, (long)(*it)+__COUNTER__,
                    "When creating tables,"
                    " $-Selector columns will not be created. "
                    "Please add the column \"" << info.Column <<
                    "\" for Selector \"" << info.Capability.substr(1) <<
                    "\" yourself.");
                LOGWARN_SA(logger, (long)(*it)+__COUNTER__,
                    "Logging will fail until then!");
                continue;
            }
            // Try to determine datatype.
            // Note: This list might be needed to be updated when new Capabilities are added
            // using new datatypes!
            assert(info.Value);
            if (!tablestring.empty())  {
                tablestring += ", ";
            }

            tablestring += SQL_ESCAPE_CHAR_OPEN + info.Column
                + SQL_ESCAPE_CHAR_CLOSE + " ";

            if (CValue<float>::IsType(info.Value)
                || CValue<double>::IsType(info.Value)) {
                LOGTRACE(logger,
                    info.Capability <<" is FLOAT and so will be column "
                    << info.Column);
                tablestring += "FLOAT";
            } else if (CValue<bool>::IsType(info.Value)) {
                LOGTRACE(logger,
                    info.Capability <<" is BOOLEAN and so will be column "
                    << info.Column);
                tablestring += "BOOLEAN";
            } else if (CValue<long>::IsType(info.Value)) {
                LOGTRACE(logger,
                    info.Capability <<" is INTEGER and so will be column "
                    << info.Column);
                tablestring += "INTEGER";
            } else if (CValue<std::string>::IsType(info.Value)) {
                LOGTRACE(logger,
                    info.Capability <<" is TEXT and so will be column "
                    << info.Column);
                tablestring += "TEXT";
            } else if (CValue<std::tm>::IsType(info.Value)) {
                LOGTRACE(logger,
                    info.Capability <<" is TIMESTAMP and so will be column "
                    << info.Column);
                tablestring += "TIMESTAMP";
            } else {
                LOGERROR(logger, "unknown datatype for " << info.Capability);
                LOGERROR(logger,
                    "Its C++ typeid is " << typeid(*(info.Value)).name());
                LOGERROR(logger,
                    "Will NOT create table. Logging will not work. Please report a bug.");
                _createtable_mode = CDBWriterHelper::cmode_no;
                return;
            }
        }

        // we can't unlock the mutex for the sql transaction ...
        // as the drop/create table could introduce a race
        // with the later insert -- we evaluated the data states earlier
        // and this could change.

        std::string tmp;
        if (_createtable_mode == CDBWriterHelper::cmode_yes_and_drop) {
            tmp = "DROP TABLE IF EXISTS ";
            tmp += SQL_ESCAPE_CHAR_OPEN + _table + SQL_ESCAPE_CHAR_CLOSE + ";";
            LOGDEBUG(logger, "Executing query: "<< tmp);
            session << tmp << cppdb::exec;
        }

        tmp = "CREATE TABLE IF NOT EXISTS ";
        tmp += SQL_ESCAPE_CHAR_OPEN + _table + SQL_ESCAPE_CHAR_CLOSE + " ("
            + tablestring + ");";
        if (_createtable_mode == CDBWriterHelper::cmode_print_statment) {
            LOGINFO(logger,
                "Your CREATE statement for table " << _table << " is:" << endl << tmp);
        } else {
            LOGDEBUG(logger, "Executing query: " << tmp);
            // NOTE: Exceptions are passed to the caller to provide error information
            session << tmp << cppdb::exec;
            LOGWARN(logger,
                "Table created. Make sure to disable table creation in the config!");
            LOGWARN(logger,
                "Otherwise, solarpowerlog might stomp on your database"
                    " the next time you start it!");
        }
        _createtable_mode = CDBWriterHelper::cmode_no;
    }

    // Part 2 -- create sql statement for adding / replacing ... data.

    // Check data availability / freshness.

    // if not everything is available and we do not doing a sparse table:
    if (!_allow_sparse && !all_available) {
        LOGDEBUG_SA(logger, LOG_SA_HASH("executequery-sparse"),
            "Only sparse dataset available.");
        return;
    } else {
        LOGDEBUG_SA(logger, LOG_SA_HASH("executequery-sparse"),
            "Dataset is not sparse (anymore).");
    }

    // on continuous mode and single mode logchangedonly is not affected by special_updated.
    if ((_mode == CDBWriterHelper::continuous
        || _mode == CDBWriterHelper::single) && _logchangedonly
        && !any_updated) {
        LOGDEBUG_SA(logger, LOG_SA_HASH("executequery-changeddata"),
            "Not logging, as data is unchanged.");
        return;
    }
    else {
        LOGDEBUG_SA(logger, LOG_SA_HASH("executequery-changeddata"),
             "Logging, as data has changed.");
    }

    // logchangedonly on cumulative mode:
    // we'll always need to insert a new row if a special has changed.
    if (_mode == CDBWriterHelper::cumulative && _logchangedonly) {
        if (!any_updated && !special_updated) {
            LOGDEBUG_SA(logger,LOG_SA_HASH("executequery-cumulative-datachange"),
                "Not logging, as data is unchanged. (cumulative)");
            return;
        }
        else {
            LOGDEBUG_SA(logger,LOG_SA_HASH("executequery-cumulative-datachange"),
                 "Logging, as data has changed. (cumulative)");
        }
    }

    // If we are still here: Seems some job to be done.

    // Case 1: continuous
    if (_mode == CDBWriterHelper::continuous) {
        // Create INSERT INTO ... statement, if not existing.
        // on sparse, we cannot use the cache
        if (!all_available) _insert_cache.clear();
        // Step one: Create sql string.
        if (_insert_cache.empty()) {
            _insert_cache = "INSERT INTO ";
            _insert_cache += SQL_ESCAPE_CHAR_OPEN + _table
                + SQL_ESCAPE_CHAR_CLOSE + " " + _GetValStringForInsert() + ';';
            LOGTRACE(logger, "SQL Statement: " << _insert_cache);
        }

        // second step: bind values.
        cppdb::statement stat;
        stat = session << _insert_cache;

        if (!_BindValues(stat)) {
            LOGDEBUG(logger, "bind for continuous failed.");
            return;
        }

        // OK, step 2 done --- step 3 is execute the query.
        _laststatementfailed = true;
        // as now the data consistency is guaranteed, we can unlock the mutex
        cma.unlock();

        // again, exceptions are passed to the caller.
        stat.exec();
        // Still here? Then it must have worked...
        _laststatementfailed = false;

        LOGTRACE(logger, "SQL: Last_insert_id=" << stat.last_insert_id()
            << " Affected rows=" << stat.affected());
        return;
    } else if (_mode == CDBWriterHelper::single
        || _mode == CDBWriterHelper::cumulative) {

        if (_mode == CDBWriterHelper::single) {
            LOGTRACE_SA(logger, __COUNTER__, "SINGLE MODE");
        }
        if (_mode == CDBWriterHelper::cumulative) {
            LOGTRACE_SA(logger, __COUNTER__, "CUMULATIVE MODE");
        }

        // single mode
        // updates a single row in the table, determined by the use of one or
        // more selectors.
        // We will first check if the dataset already exists to be able to select
        // either a UPDATE of INSERT statement. This needs only to be done
        // once, as once UPDATEd we can always UPDATE.

        // cumulative mode
        // very similar to the single mode (indeed we could merge them :))
        // but there are additional selectors with non-static selecting values
        // This are "%"-special types, but the prefix is "!" to differentiate.

        // Step 1) Assemble data ppart
        std::string cols, selectors;

        cols = _GetValStringForUpdate();

        if (cols.empty()) {
            LOGERROR_SA(logger, __COUNTER__, "No columns found for table "
                << _table);
            return;
        }

        for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
            Cdbinfo &info = **it;
            if (info.Capability[0] == '$'
                || (_mode == CDBWriterHelper::cumulative
                    && info.Capability[0] == '!')) {
                // Selector
                if (!selectors.empty()) {
                    selectors += " AND ";
                }
                selectors += SQL_ESCAPE_CHAR_OPEN + info.Column
                    + SQL_ESCAPE_CHAR_CLOSE + "=?";
                //selectors += info.Capability.substr(1); // len of capability ensured in cfg check.
            }
        }

        if (selectors.empty()) {
            LOGERROR_SA(logger, __COUNTER__, "no selectors found for table "
                << _table);
            return;
        }

        std::string query_common = SQL_ESCAPE_CHAR_OPEN + _table
            + SQL_ESCAPE_CHAR_CLOSE + " SET " + cols + " ";
        std::string update_query = "UPDATE " + query_common + "WHERE "
            + selectors + ";";

        LOGTRACE(logger, "Update-query=" << update_query);

        cppdb::statement stat;
        stat = session << update_query;

        if (!_BindValues(stat)) {
            LOGDEBUG_SA(logger, __COUNTER__, "bind failed (cumulative/single).");
            return;
        }

        // now binding the selectors.
        for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
            Cdbinfo &info = **it;
            if (info.Capability[0] == '$') {
                // Selector
                stat.bind(info.Capability.substr(1));
            } else if (_mode == CDBWriterHelper::cumulative
                && info.Capability[0] == '!') {
                if (!_BindSingleValue(stat, info)) {
                    LOGDEBUG_SA(logger, __COUNTER__,
                        "Binding single value failed");
                    return;
                }
            }
        }

        LOGTRACE(logger, "Binding done.");

        LOGTRACE(logger, "Trying UPDATE query");
        _laststatementfailed = true;
        stat.exec();
        _laststatementfailed = false;
        LOGDEBUG(logger,
            "UPDATE tried. " << stat.affected() << " affected row(s)");
        if (stat.affected() == 0) {
            LOGDEBUG(logger, "no affected rows. Trying INSERT instead.");
            stat.clear();
            // we try now INSERT INTO table (col1,col2,col3) VALUES (1,2,3);
            // as above in the continuous mode.
            std::string insert_query = "INSERT INTO [" + _table + "] "
                + _GetValStringForInsert(true) + ';';
            LOGTRACE(logger, "SQL Statement: " << insert_query);

            stat = session << insert_query;

            LOGTRACE(logger, "Binding values and selectors ...");
            if (!_BindValues(stat, true)) {
                LOGDEBUG_SA(logger, __COUNTER__, "Bind failed (single, insert)");
                return;
            }

           LOGDEBUG(logger, "About to execute INSERT Query (single mode)");
            _laststatementfailed = true;
            stat.exec();
            _laststatementfailed = false;
            LOGDEBUG(logger,
                "Insert tried. " << stat.affected() << " rows affected." << "Last ID=" << stat.last_insert_id());
            return;
        }
    }
    return;
}

std::string CDBWriterHelper::_GetValStringForUpdate(void)
{
    std::string ret;
    std::vector<class Cdbinfo*>::iterator it;
    for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
        Cdbinfo &info = **it;
        if (info.Capability[0] == '$') {
            continue;
        } else {
            if (!ret.empty()) ret += ",";
            if (info.Value) {
                ret += SQL_ESCAPE_CHAR_OPEN + info.Column
                    + SQL_ESCAPE_CHAR_CLOSE + "=?";
            }
        }
    }
    return ret;
}

//(col1,col2,col3) VALUES (?,?,?)

std::string CDBWriterHelper::_GetValStringForInsert(bool with_selector)
{

    std::vector<class Cdbinfo*>::iterator it;
    std::string vals, cols;
    for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
        Cdbinfo &info = **it;
        if (info.Value || (with_selector && info.Capability[0] == '$')) {
            if (!cols.empty()) cols += ',';
            if (!vals.empty()) vals += ',';
            cols += SQL_ESCAPE_CHAR_OPEN + info.Column + SQL_ESCAPE_CHAR_CLOSE;
            vals += "?";
        };
    }
    return '(' + cols + ") VALUES (" + vals + ')';
}

bool CDBWriterHelper::_BindValues(cppdb::statement &stat, bool with_selector)
{

    std::vector<class Cdbinfo*>::iterator it;
    for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
        Cdbinfo &info = **it;
        if (with_selector) {
            if (info.Capability[0] == '$') {
                stat.bind(info.Capability.substr(1));
                continue;
            }
        }
        if (!info.Value) continue;

        if (!_BindSingleValue(stat, info)) return false;

    }
    return true;
}

bool CDBWriterHelper::_BindSingleValue(cppdb::statement &stat, Cdbinfo &info)
{

    // Bind the values considering their datatypes.
    if (CValue<float>::IsType(info.Value)) {
        stat.bind((double)((CValue<float> *)info.Value)->Get());
    } else if (CValue<double>::IsType(info.Value)) {
        stat.bind(((CValue<double> *)info.Value)->Get());
    } else if (CValue<bool>::IsType(info.Value)) {
        stat.bind(((CValue<bool> *)info.Value)->Get());
    } else if (CValue<long>::IsType(info.Value)) {
        stat.bind(((CValue<long> *)info.Value)->Get());
    } else if (CValue<std::string>::IsType(info.Value)) {
        stat.bind(((CValue<std::string> *)info.Value)->Get());
    } else if (CValue<std::tm>::IsType(info.Value)) {
        stat.bind(((CValue<std::tm> *)info.Value)->Get());
    } else {
        LOGERROR_SA(logger, __COUNTER__, "unknown datatype for " << info.Capability);
        LOGERROR_SA(logger, __COUNTER__, "Its C++ typeid is " << typeid(*(info.Value)).name());
        LOGERROR_SA(logger, __COUNTER__, "Cannot put data to database.");
        return false;
    }
    return true;
}

#endif
