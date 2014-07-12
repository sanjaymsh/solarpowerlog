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
#include <Inverters/interfaces/ICapaIterator.h>

/** \file CDBWriterFilter.cpp
 *
 *  Created on: Jun 28, 2014
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef  HAVE_FILTER_DBWRITER

#include "DataFilters/DBWriter/CDBWriterFilter.h"

#include "configuration/CConfigHelper.h"
#include "interfaces/CWorkScheduler.h"

#include "Inverters/Capabilites.h"
#include "patterns/CValue.h"

using namespace libconfig;

CDBWriterFilter::CDBWriterFilter( const std::string & name,
	const std::string & configurationpath ) :
	IDataFilter(name, configurationpath)
{
    _datavalid = false;

	// Schedule the initialization and subscriptions later...
	ICommand *cmd = new ICommand(CMD_INIT, this);
	Registry::GetMainScheduler()->ScheduleWork(cmd);

	// We do not anything on these capabilities, so we remove our list.
	// any cascaded filter will automatically use the parents one...
	CCapability *c = IInverterBase::GetConcreteCapability(
		CAPA_INVERTER_DATASTATE);
	CapabilityMap.erase(CAPA_INVERTER_DATASTATE);
	delete c;

	// Also we wont fiddle with the caps requiring our listeners to unsubscribe.
	// They also should get that info from our base.
	c = IInverterBase::GetConcreteCapability(CAPA_CAPAS_REMOVEALL);
	CapabilityMap.erase(CAPA_CAPAS_REMOVEALL);
	delete c;

	// Also we do not update capas on our own..
    c = IInverterBase::GetConcreteCapability(CAPA_CAPAS_UPDATED);
    CapabilityMap.erase(CAPA_CAPAS_UPDATED);
    delete c;


    Registry::GetMainScheduler()->RegisterBroadcasts(this);
}

CDBWriterFilter::~CDBWriterFilter()
{
    std::vector<CDBWriterHelper*>::iterator it;
    for (it = _dbwriterhelpers.begin(); it != _dbwriterhelpers.end(); it++) {
        delete *it;
    }
}

bool CDBWriterFilter::CheckConfig()
{
    CConfigHelper hlp(configurationpath);
    bool fail = false;
    std::string str;

    fail |= !hlp.CheckAndGetConfig("datasource", Setting::TypeString, str);
    IInverterBase *dsource = Registry::Instance().GetInverter(str);
    if (!dsource) {
        LOGERROR(logger, " Cannot find datassource with the name "
            << str );
        fail = true;
    }
    base = dsource;

    // What settings does a database writer needs?

    // CPPDB Settings
    // The library provides a connection string.
    // However, to keep all options available, we query the components and assemble it ourself.
    // (There will be an optional "parameters" string for special uses...)

    // Database
    // Soci needs the engine and a connection string. (engine dependent)
    // We need also the database, the table to act on
    // Also we'll need a list / table for all to be logged capabilities and its columns in the database
    // write the timestamp where

    // Handling
    // How often should we write to the database. Time-based, value based?

    // the db engine. e.g mysql, sqlite3, postgresql or odbc
    // (for odbc you'll need to provide all options in db_cppdb_options yourself...)
    // if you use "custom" here, then solarpowerlog will just use db_cppdb_options as connection string.

    std::string db_type, db_host, db_port, db_unixsocket, db_mode;
    std::string db_user, db_passwd, db_database, db_cppdb_options;


    // The driver to be used.
    fail |= !hlp.CheckAndGetConfig("db_type", Setting::TypeString, db_type);

    // the host (optional, as not every db needs it)
    fail |= !hlp.CheckAndGetConfig("db_host", Setting::TypeString, db_host,
        true);

    // for sqlite3 the mode the db should be opened. (create, readwrite)
    // the 3rd option readonly makes no sense here
    fail |= !hlp.CheckAndGetConfig("db_mode", Setting::TypeString, db_mode,
        true);

    // port (as string), optional as not every db needs it
    fail |= !hlp.CheckAndGetConfig("db_port", Setting::TypeString, db_port,
        true);

    // (for mysql) unix-socket to be used.
    fail |= !hlp.CheckAndGetConfig("db_unixsocket", Setting::TypeString,
        db_unixsocket, true);

    if (!db_port.empty() && !db_unixsocket.empty()) {
        fail = true;
        LOGERROR(logger,
            "both db_port and db_unixsocket cannot be used at the same time.");
    }

    // username optional as not every db needs it
    fail |= !hlp.CheckAndGetConfig("db_user", Setting::TypeString, db_user,
        true);

    // password (as string), optional as not every db needs it
    fail |= !hlp.CheckAndGetConfig("db_password", Setting::TypeString,
        db_passwd, true);

    // database to be used (as string), optional as not every db needs it
    fail |= !hlp.CheckAndGetConfig("db_database", Setting::TypeString,
        db_database, true);

    // option string which will be appended to the connection string.
    // See your backend config http://cppcms.com/sql/cppdb/backendref.html
    // and http://cppcms.com/sql/cppdb/connstr.html for cppdb options.
    fail |= !hlp.CheckAndGetConfig("db_cppdb_options", Setting::TypeString,
        db_cppdb_options, true);

    if (fail) return false;

    bool add_semicolon = false;
    std::string dbgstring;
    // Generate the connection string.
    if (db_type == "mysql") {
        _connectionstring = "mysql:";
        if (!db_host.empty()) {
            _connectionstring += "host=\'" + db_host + "\'";
            add_semicolon = true;
        }
        if (!db_user.empty()) {
            if (add_semicolon) _connectionstring += ";";
            _connectionstring += "user=\'" + db_user + "\'";
            add_semicolon = true;
        }
        if (!db_database.empty()) {
            if (add_semicolon) _connectionstring += ";";
            _connectionstring += "database=\'" + db_database + "\'";
            add_semicolon = true;
        }
        if (!db_port.empty()) {
            if (add_semicolon) _connectionstring += ";";
            _connectionstring += "port=\'" + db_port + "\'";
            add_semicolon = true;
        }
        if (!db_unixsocket.empty()) {
            if (add_semicolon) _connectionstring += ";";
            _connectionstring += "unix_socket=\'" + db_unixsocket + "\'";
            add_semicolon = true;
        }
        dbgstring = _connectionstring;
        if (!db_passwd.empty()) {
            if (add_semicolon) _connectionstring += ";";
            dbgstring = _connectionstring + "password=\'***\'";
            _connectionstring += "password=\'" + db_passwd + "\'";
            add_semicolon = true;
        }
        if (!db_cppdb_options.empty()) {
            std::string tmp;
            if (add_semicolon) tmp = ";";
            tmp += db_cppdb_options;
            _connectionstring += tmp;
            dbgstring += tmp;
            add_semicolon = true;
        }
        if (!db_mode.empty()) {
            LOGFATAL(logger,
                db_type << " does not support the db_mode parameter.");
            fail = true;
        }
    }
    if (db_type == "postgresql") {
        _connectionstring = "postgresql:";
        if (!db_host.empty()) {
            _connectionstring += "host=\'" + db_host + "\'";
            add_semicolon = true;
        }
        if (!db_user.empty()) {
            if (add_semicolon) _connectionstring += ";";
            _connectionstring += "user=\'" + db_user + "\'";
            add_semicolon = true;
        }
        if (!db_database.empty()) {
            if (add_semicolon) _connectionstring += ";";
            _connectionstring += "dbname=\'" + db_database + "\'";
            add_semicolon = true;
        }
        if (!db_port.empty()) {
            if (add_semicolon) _connectionstring += ";";
            _connectionstring += "port=\'" + db_port + "\'";
            add_semicolon = true;
        }
        if (!db_unixsocket.empty()) {
            LOGFATAL(logger, "db_unixsocket is not supported for postgressql");
            fail = true;
        }
        dbgstring = _connectionstring;
        if (!db_passwd.empty()) {
            if (add_semicolon) _connectionstring += ";";
            dbgstring = _connectionstring + "password=\'***\'";
            _connectionstring += "password=\'" + db_passwd + "\'";
            add_semicolon = true;
        }
        if (!db_cppdb_options.empty()) {
            std::string tmp;
            if (add_semicolon) tmp = ";";
            tmp += db_cppdb_options;
            _connectionstring += tmp;
            dbgstring += tmp;
            add_semicolon = true;
        }
        if (!db_mode.empty()) {
            LOGFATAL(logger,
                db_type << " does not support the db_mode parameter.");
            fail = true;
        }
    }
    if (db_type == "sqlite3") {
        _connectionstring = "sqlite3:";
        if (!db_host.empty()) {
            LOGFATAL(logger, db_type << " does not support the db_host parameter.");
            fail = true;
        }
        if (!db_user.empty()) {
            LOGFATAL(logger, db_type << " does not support the db_user parameter.");
            fail = true;
        }
        if (!db_database.empty()) {
            if (add_semicolon) _connectionstring += ";";
            _connectionstring += "db=\'" + db_database + "\'";
            add_semicolon = true;
        }
        if (!db_mode.empty()) {
            if (add_semicolon) _connectionstring += ";";
            _connectionstring += "mode=\'" + db_mode + "\'";
            add_semicolon = true;
        }
        if (!db_port.empty()) {
            LOGFATAL(logger, db_type << " does not support the db_port parameter.");
            fail = true;
        }
        if (!db_unixsocket.empty()) {
            LOGFATAL(logger,
                db_type << " does not support the db_unixsocket parameter.");
            fail = true;
        }
        if (!db_passwd.empty()) {
            LOGFATAL(logger,
                db_type << " does not support the db_password parameter.");
            fail = true;
        }
        if (!db_cppdb_options.empty()) {
            if (add_semicolon) _connectionstring += ";";
            _connectionstring += db_cppdb_options;
            add_semicolon = true;
        }
        dbgstring = _connectionstring;
    }
    if (db_type == "odbc" || db_type == "custom") {
        if (db_type == "odbc") _connectionstring = "odbc:";
        if (!db_host.empty() || !db_user.empty() || !db_database.empty()
            || !db_mode.empty() || !db_port.empty() || !db_unixsocket.empty()
            || !db_passwd.empty() || db_cppdb_options.empty()) {
            LOGERROR(logger,
                db_type << " please use db_cppdb_options to specify the connection string.");
            fail = true;
        }
        _connectionstring += db_cppdb_options;
        dbgstring = _connectionstring;
    }

    LOGTRACE(logger, "DBWriter: connectionstring for CppDB is: " << dbgstring);

    // Checking config for the tables aggregate
    int i=0;
    CDBWriterHelper *dbwh = NULL;

    do {
        std::string table, mode, createmode;
        bool logchangedonly;
        float logevery;
        logchangedonly = false;
        logevery = 0;
        dbwh = NULL;

        hlp = CConfigHelper(configurationpath, "db_jobs", i);

        if (!hlp.isExisting()) {
            if (i == 0) {
                LOGFATAL(logger, "db_jobs missing in configuration.");
            }
            break;
        }

        if (!hlp.CheckAndGetConfig("db_table", Setting::TypeString, table)) {
            LOGFATAL(logger, "db_table missing in entry number "
                << i << " of the tables list.");
            fail = true;
            i++; continue;
        }

        LOGINFO(logger, "Parsing configuration for table " << table);

        if (!hlp.CheckAndGetConfig("db_create_table", Setting::TypeString, createmode, true)) {
            LOGFATAL(logger, "db_create_table is of wrong type for table " << table);
            fail = true;
        } else if (!createmode.empty()) {
            if (createmode == "YES") {
                LOGWARN(logger, "db_create_table is YES -- will create this table later: " << table);

            } else
            if (createmode == "YES-WIPE-MY-DATA") {
                LOGWARN(logger, "db_create_table is YES-WIPE-MY-DATA -- will *DESTROY* and create this table later: " << table);

            }
            else {
                LOGWARN(logger, "db_create_table is neither YES nor YES-WIPE-MY-DATA. Ignored");
                createmode ="";
            }
        }

        if (!hlp.GetConfig("db_operation_mode", mode)) {
            fail = true;
        } else {
            if (!(mode == "continuous" || mode == "single"
                || mode == "cumulative")) {
                LOGFATAL(logger,
                    "Setting operation_mode for table " << table <<
                    " must be either continuous, single or cumulative, not " << mode);
                fail = true;
            } else {
                LOGDEBUG(logger,
                    "Setting operation_mode for table " << table <<
                    " is " << mode);
            }
        }

        if (!hlp.CheckConfig("db_logchangedonly", Setting::TypeBoolean)) {
            fail = true;
        } else {
            hlp.GetConfig("db_logchangedonly", logchangedonly, false);
        }

        bool allow_sparse;
        if (!hlp.CheckConfig("db_allowsparse", Setting::TypeBoolean)) {
            fail = true;
        } else {
            hlp.GetConfig("db_allowsparse", allow_sparse, false);
        }


#warning FIXME  logevery is optional and should be derived from the inverter if not specified.
        if (!hlp.CheckConfig("db_logevery", Setting::TypeFloat)) {
            fail = true;
        } else {
            hlp.GetConfig("db_logevery", logevery);
        }

        hlp = CConfigHelper(hlp.GetCfgPath(),"db_layout");
        if (!hlp.isExisting()) {
            LOGFATAL(logger, "Configuration error: Entry db_layout missing for table " << table);
            i++; continue;
        }

        dbwh = new CDBWriterHelper(base, logger, table, mode, createmode,
            logchangedonly, logevery, allow_sparse);

        int j=0;
        bool k = true;
        // Iterate over all datasets to be logged.
        do {
            std::string capa,column;
            capa.clear(); column.clear();
            CConfigHelper hlp2(hlp.GetCfgPath(),j);
            k = hlp2.isExisting();

            if (!k) break;

            if (!(hlp.GetConfigArray(j, 0, capa) && hlp.GetConfigArray(j, 1, column))) {
                LOGFATAL(logger, "Config Error in entry " << j
                    << " of " << table << "::" << "db_layout");
                fail = true;
                j++; continue;
            }
            LOGDEBUG(logger, "j=" << j << " capa=" << capa <<
                " column=" << column);

            if (capa.empty()) {
                LOGFATAL(logger, "Capability in entry " << j
                    << " of " << table << "::" << "db_layout must not be empty");
               fail = true;
               j++; continue;
            }
            if (column.empty()) {
                LOGFATAL(logger, "Column in entry " << j
                    << " of " << table << "::" << "db_layout must not be empty");
               fail = true;
               j++; continue;
            }

            if (!dbwh->AddDataToLog(capa,column)) {fail = true;}
            j++;
        } while(k);

        if (!j) {
            LOGFATAL(logger, "no layout for table " << table);
            fail = true;
        }
        _dbwriterhelpers.push_back(dbwh);

        i++;
    }
    while(true);

    return !fail;
}

void CDBWriterFilter::Update( const IObserverSubject *subject )
{
    // TODO check if we can completely empty this member function
    // NOTE: The Observer-Pattern-Handling is delegated to the helper claa
    // CDBWriterHelper

    assert(subject);
    CCapability *cap = (CCapability *)subject;

    // Datastate changed.
    if (cap->getDescription() == CAPA_INVERTER_DATASTATE) {
        _datavalid = ((CValue<bool> *)cap->getValue())->Get();
        return;
    }

    // Unsubscribe plea -- we do not offer this Capa, our customers will
    // ask our base directly.
    if (cap->getDescription() == CAPA_CAPAS_REMOVEALL) {
        auto_ptr<ICapaIterator> it(base->GetCapaNewIterator());
        while (it->HasNext()) {
            pair<string, CCapability*> cappair = it->GetNext();
            cap = (cappair).second;
            cap->UnSubscribe(this);
        }
        return;
    }
}

void CDBWriterFilter::ScheduleCyclicWork(void)
{
    ICommand *ncmd = NULL;
    struct timespec ts;
    std::vector<CDBWriterHelper*>::iterator it;

    for (it = _dbwriterhelpers.begin(); it != _dbwriterhelpers.end(); it++) {
        ncmd = new ICommand(CMD_CYCLIC, this);
        ncmd->addData("DB_WORK", *it);
        ts.tv_sec = (*it)->_logevery;
        ts.tv_nsec = ((*it)->_logevery - ts.tv_sec) * 1e9;
        Registry::GetMainScheduler()->ScheduleWork(ncmd, ts);
    }
}

void CDBWriterFilter::ExecuteCommand( const ICommand *cmd )
{

//    LOGERROR(logger, __PRETTY_FUNCTION__ << " Warning not finished / implemented");

	switch (cmd->getCmd()) {

        case CMD_INIT: {
            DoINITCmd(cmd);
            ScheduleCyclicWork();
        }
        break;

        case CMD_CYCLIC: {
            DoCYCLICmd(cmd);

            CDBWriterHelper* helper = NULL;

            try {
                helper = boost::any_cast<CDBWriterHelper*>(
                    cmd->findData("DB_WORK"));

            } catch (...) {
                LOGDEBUG(logger,
                    __PRETTY_FUNCTION__ << "Unexpected exception while getting "
                        "object out of cmd");
            }

            assert(helper);

            LOGTRACE(logger, "now handling: " << helper->GetTable());

            if (!_sqlsession.is_open()) {
                LOGDEBUG(logger, "Trying to open database.");
                try {
                    _sqlsession.open(_connectionstring);
                } catch (std::exception const &e) {
                    LOGWARN(logger, "Exception while opening database: "
                        << e.what());
                }
            }

            if (_sqlsession.is_open()) {
                try {
                    helper->ExecuteQuery(_sqlsession);
                } catch (const std::exception &e) {
                    LOGWARN(logger, "Exception while handling database access:" << e.what() );
                    LOGWARN(logger, "Closing DB connection to try error recovery.");
                    _sqlsession.close();
                }
            }

            ICommand *ncmd = new ICommand(CMD_CYCLIC, this);
            struct timespec ts;
            ncmd->addData("DB_WORK", helper);
            ts.tv_sec = helper->_logevery;
            ts.tv_nsec = (helper->_logevery - ts.tv_sec) * 1e9;
            Registry::GetMainScheduler()->ScheduleWork(ncmd, ts);
        }
        break;

        case CMD_BRC_SHUTDOWN:
            // shutdown requested, we will terminate soon.
            try {
                if (_sqlsession.is_open()) _sqlsession.close();
            } catch (std::exception const &e) {
                LOGWARN(logger, " Exception while closing sqlsession. "
                    << e.what());

            }
        break;
    }
}

void CDBWriterFilter::DoINITCmd( const ICommand * )
{
    // Subscribe to base capabilities.
    // probably not needed!
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

    return;
}

void CDBWriterFilter::DoCYCLICmd( const ICommand * )
{
 //   LOGERROR(logger, __PRETTY_FUNCTION__ << " not implemented");
    return;

}

#endif
