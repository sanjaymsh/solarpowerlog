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

    Registry::GetMainScheduler()->RegisterBroadcasts(this);
}

CDBWriterFilter::~CDBWriterFilter()
{
}

bool CDBWriterFilter::CheckConfig()
{
    CConfigHelper hlp(configurationpath);
    bool fail = false;
    std::string db_type, db_host, db_port, db_unixsocket, db_mode;
    std::string db_user, db_passwd, db_database, db_cppdb_options;

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
            LOGERROR(logger,
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
            LOGERROR(logger, "db_unixsocket is not supported for postgressql");
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
            LOGERROR(logger,
                db_type << " does not support the db_mode parameter.");
            fail = true;
        }
    }
    if (db_type == "sqlite3") {
        _connectionstring = "sqlite3:";
        if (!db_host.empty()) {
            LOGERROR(logger, db_type << " does not support the db_host parameter.");
            fail = true;
        }
        if (!db_user.empty()) {
            LOGERROR(logger, db_type << " does not support the db_user parameter.");
            fail = true;
        }
        if (!db_database.empty()) {
            if (add_semicolon) _connectionstring += ";";
            _connectionstring += "database=\'" + db_database + "\'";
            add_semicolon = true;
        }
        if (!db_mode.empty()) {
            if (add_semicolon) _connectionstring += ";";
            _connectionstring += "mode=\'" + db_mode + "\'";
            add_semicolon = true;
        }
        if (!db_port.empty()) {
            LOGERROR(logger, db_type << " does not support the db_port parameter.");
            fail = true;
        }
        if (!db_unixsocket.empty()) {
            LOGERROR(logger,
                db_type << " does not support the db_unixsocket parameter.");
            fail = true;
        }
        if (!db_passwd.empty()) {
            LOGERROR(logger,
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

    return !fail;

}

void CDBWriterFilter::Update( const IObserverSubject *subject )
{
    LOGERROR(logger, __PRETTY_FUNCTION__ << " not implemented");
    return;

#if 0
	assert (subject);
	CCapability *c, *cap = (CCapability *) subject;

	// Datastate changed.
	if (cap->getDescription() == CAPA_INVERTER_DATASTATE) {
		this->datavalid = ((CValue<bool> *) cap->getValue())->Get();
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

	// propagate "caps updated"
	if (cap->getDescription() == CAPA_CAPAS_UPDATED) {
		c = IInverterBase::GetConcreteCapability(CAPA_CAPAS_UPDATED);
		*(CValue<bool> *) c->getValue()
			= *(CValue<bool> *) cap->getValue();
		c->Notify();
		capsupdated = true;
		return;
	}
#endif

}

void CDBWriterFilter::ExecuteCommand( const ICommand *cmd )
{

    LOGERROR(logger, __PRETTY_FUNCTION__ << " not implemented");
     return;

	switch (cmd->getCmd()) {

	case CMD_INIT:
	{
		DoINITCmd(cmd);

		ICommand *ncmd = new ICommand(CMD_CYCLIC, this);
		struct timespec ts;
		// Set cyclic timer to the query interval.
		ts.tv_sec = 5;
		ts.tv_nsec = 0;

		CCapability *c = GetConcreteCapability(
			CAPA_INVERTER_QUERYINTERVAL);
		if (c && CValue<float>::IsType(c->getValue())) {
			CValue<float> *v = (CValue<float> *) c->getValue();
			ts.tv_sec = v->Get();
			ts.tv_nsec = ((v->Get() - ts.tv_sec) * 1e9);
		} else {
			LOGINFO(logger,
				"INFO: The associated inverter does not specify the "
				"queryinterval. Defaulting to 5 seconds");
		}

		Registry::GetMainScheduler()->ScheduleWork(ncmd, ts);

	}
		break;

	case CMD_CYCLIC:
	{
		DoCYCLICmd(cmd);

		// Set cyclic timer to the query interval.
		ICommand *ncmd = new ICommand(CMD_CYCLIC, this);
		struct timespec ts;
		ts.tv_sec = 5;
		ts.tv_nsec = 0;

		CCapability *c = GetConcreteCapability(
			CAPA_INVERTER_QUERYINTERVAL);
		if (c && CValue<float>::IsType(c->getValue())) {
			CValue<float> *v = (CValue<float> *) c->getValue();
			ts.tv_sec = v->Get();
			ts.tv_nsec = ((v->Get() - ts.tv_sec) * 1e9);
		}

		Registry::GetMainScheduler()->ScheduleWork(ncmd, ts);

	}
		break;

	case CMD_BRC_SHUTDOWN:
            // shutdown requested, we will terminate soon.
        break;
    }
}

void CDBWriterFilter::DoINITCmd( const ICommand * )
{
    LOGERROR(logger, __PRETTY_FUNCTION__ << " not implemented");
    return;
}

void CDBWriterFilter::DoCYCLICmd( const ICommand * )
{
    LOGERROR(logger, __PRETTY_FUNCTION__ << " not implemented");
    return;

#if 0
	bool compact_file, flush_after_write;
	std::string format;

	CConfigHelper cfg(configurationpath);
	cfg.GetConfig("format_timestamp", format, std::string("%Y-%m-%d %T"));
	cfg.GetConfig("compact_csv", compact_file, false);
	cfg.GetConfig("flush_file_buffer_immediatly", flush_after_write, false);

	std::stringstream ss;

	/* Check for data validty. */
	if (!datavalid) {
		return;
	}

	/* check if CSV-Header needs to be re-emitted.*/
	if (capsupdated || !headerwritten) {
		capsupdated = false;
		if (CMDCyclic_CheckCapas()) {
			headerwritten = false;
		}
	}

	/* check if file is ready */
	if (!file.is_open()) {
		return;
	}

	/* output CSV Header*/
	if (!headerwritten) {
		last_line.clear();
		bool first = true;
		list<string>::const_iterator it;
		for (it = CSVCapas.begin(); it != CSVCapas.end(); it++) {
			if (!first) {
				ss << ",";
			} else {
				ss << "Timestamp,";
			}
			first = false;
			ss << *(it);
		}
		// CSV after RFC 4180 requires CR LF
		file << ss.str() << (char) 0x0d << (char) 0x0a;
		CCapability *cap = GetConcreteCapability(CAPA_CSVDUMPER_LOGGEDCAPABILITES);
		assert(cap);
		((CValue<std::string> *)cap->getValue())->Set(ss.str());
		cap->Notify();
		ss.str("");
		headerwritten = true;
	}

	/* finally, output data. */

	// make timestamp
	boost::posix_time::ptime n =
		boost::posix_time::second_clock::local_time();

	// assign facet only to a temporay stringstream.
	// this avoids having a persistent object.
	/// time_facet for the formating of the string
	boost::posix_time::time_facet *facet = new boost::posix_time::time_facet(format.c_str());
	ss.imbue(std::locale(ss.getloc(), facet));
	ss << n;
	file << ss.str();
	ss.str("");

	// note: do not delete the facet. This is done by the locale.
	// See: http://rhubbarb.wordpress.com/2009/10/17/boost-datetime-locales-and-facets/
	// (the locale will delete the object, so there is no leak. If we would
	// delete, this crashes.)

	list<string>::const_iterator it;
	CCapability *c;
	IValue *v;
	for (it = CSVCapas.begin(); it != CSVCapas.end(); it++) {
		ss << ",";
		c = base->GetConcreteCapability(*it);
		if (c) {
			v = c->getValue();
			string tmp = (string) *v;

			if (string::npos != tmp.find('"')) {
				string t2 = tmp;
				size_t t;
				while (string::npos != (t = t2.find('"'))) {
					tmp = t2.substr(0, t);
					tmp += '"';
					t2 = t2.substr(t, string::npos);
				}
				tmp += t2;
			}

			if (string::npos != tmp.find(',') || string::npos
				!= tmp.find("\x0d\x0a")) {
				ss << '"' << tmp << '"';
			} else {
				ss << tmp;
			}

		} else {
			// file << ' ';
		}
	}

	if ( !compact_file ||  ss.str() != last_line) {
		file << ss.str() << (char) 0x0d << (char) 0x0a;
		last_line = ss.str();
		if (flush_after_write)
			file << flush;
	}
#endif
}

#endif
