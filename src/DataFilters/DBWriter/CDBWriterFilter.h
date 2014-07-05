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

/** \file CDBWriterFilter.h
 * \date Jul 28, 2014
 * \author Tobias Frost
 *
 * TODO Dokumentation is missing!!!!!
 */

#ifndef CDBWriterFilter_H_
#define CDBWriterFilter_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_FILTER_DBWRITER

#include "DataFilters/interfaces/IDataFilter.h"

#warning carve out this helper class into own file!
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
class CDBWriterHelper
{

public:

    CDBWriterHelper(const ILogger &parent, const std::string &table,
        const std::string &mode, bool logchangedonly, float logevery)
    {
        logger.Setup(parent.getLoggername(),"CDBWriterHelper(" + table + ")");
        _table = table;
        _table_sanizited = false;
        _logevery = logevery;
        _logchangedonly = logchangedonly;

        if (mode == "continuous") {
            _mode = CDBWriterHelper::continuous;
        } else if (mode == "single") {
            _mode = CDBWriterHelper::single;
        } else if (mode == "cumulative") {
            _mode = CDBWriterHelper::cumulative;
        }

    }

    /// Add the tuple Capability, Column to the "should be logged information"
    /// Returns "FALSE" if the combination of Capabilty and Column is alreaedy there.
    bool AddDataToLog(const std::string &Capability, const std::string &Column)
    {

        assert(!Capability.empty());
        assert(!Column.empty());
        std::vector<Cdbinfo>::iterator it;

#warning TODO check capabilities for "special" names and reject if unknown special names are used.

        // Check if we need to complain on the tablename...
        if (!_table_sanizited && !issane(_table)) {
            LOGFATAL(logger, "Tablename is not sane: " << _table);
            return false;
        }
        _table_sanizited = true;

        if(!issane(Column)) {
            LOGFATAL(logger, "Columname is not sane: " << Column);
            return false;
        }

        for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
            if ((*it).Column == Column) {
                LOGFATAL(logger,
                    "Table " << _table << ": Column " << Column <<" already used as target:");
                return false;
            }
        }

        LOGDEBUG(logger, "\"" << Capability << "\" will be logged to column \""
            << Column << "\"");
        _dbinfo.push_back(Cdbinfo(Capability, Column));
        return true;
    }

    float _logevery;
    bool _logchangedonly;

private:

    bool issane(const std::string s) {
        // needle is selected from mysqli.real-escape-string
        // I added the brackets, which might be overkill...
        const char *needle = "\"\'`[]\0\r\n\x1a%";
        if (std::string::npos != s.find_first_of(needle, 0, sizeof(needle))) {
            return false;
        }
        return true;
    }

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
            wasSeen(wasSeen)
    {};

    std::string Capability;
    std::string Column;
    bool wasUpdated;
    bool wasSeen;
    };

    ILogger logger;

    std::vector<Cdbinfo> _dbinfo;

    std::string _table;
    omode _mode;

    bool _table_sanizited;

};


/** This class implements a logger to write the data to a CSV File
 *
 * Please see \ref DLCSV_Description for configuration, etc.
 */

class CDBWriterFilter : public IDataFilter
{

protected:
	friend class IDataFilterFactory;
	CDBWriterFilter( const std::string &name, const std::string & configurationpath );

public:
	virtual ~CDBWriterFilter();

	virtual bool CheckConfig();

	virtual void  Update( const IObserverSubject *subject );

	virtual void ExecuteCommand( const ICommand *cmd );

private:

	/** Do the initialization of the module
	*/
	void DoINITCmd (const ICommand *);

	/** does the actual work:
	 */
	void DoCYCLICmd(const ICommand *);

    enum Commands
    {

        CMD_BRC_SHUTDOWN = BasicCommands::CMD_BRC_SHUTDOWN,
        CMD_INIT = BasicCommands::CMD_USER_MIN,
        CMD_CYCLIC
    };

    std::string _connectionstring;
    std::string _table;

    std::vector<CDBWriterHelper*> _dbwriterhelpers;

};

#endif

#endif /* CDBWriterFilter_H_ */
