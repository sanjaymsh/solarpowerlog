/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licensed
 as described in the file "COPYING".

 For this file (CCSVOutputFilter.h), the license terms are:

 You can redistribute it and/or  modify it under the terms of the GNU Lesser
 General Public License (LGPL) as published by the Free Software Foundation;
 either version 3 of the License, or (at your option) any later version.

 This programm is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/** \file CCSVOutputFilter.h
 * \date Jul 1, 2009
 * \author Tobias Frost
*/

/**
 * \page CVSDataLogger [LOGGER] CVSDataLogger: Logging to CSV Files
 *
 * \section DLCSV_Description Overview
 * The CSV Data Logger takes some or all data and writes it to a regular comma-
 * sepearated-file.
 *
 * The data to be logged can be selected by specifing the identifiers or by
 * all data.
 *
 * Solarpowerlog honors the RFC 4180. However, some feature, like the "log all"
 * feature causes that for example the number of columns changes during runtime.
 * (However, patches will be accepted to fix this: For example, on a new feature,
 * one could just reparse the file and add the missing datas.
 *
 * Unavailable datas will be set to empty value.
 *
 * To increase the use of the logfile, a (ISO 8601)-like timestamp
 * will be inserted as the first column.
 *
 * \section DLCSV_Configuration Configuration
 *
 * As every logger, the CSV Logger is configured using the Loggers Section.
 *
 * <TABLE border=1 summary="CSV Logger Configuration Options" >
 * <tr>
 * 	<td> <b>Option</b> </td>
 * 	<td> <b>Type</b> </td>
 * 	<td> <b>Mandatory</b> </td>
 * 	<td> <b>Default Value</b></td>
 *      <td> <b>Description </b></td>
 *</tr>
 * <tr>
 * 	<td> name </td>
 * 	<td> string </td>
 * 	<td> yes </th>
 * 	<td> &nbsp; </td>
 *      <td> Names the Logger. Used to identify the logger, and alos needed
 *           if the logger should be chained.  </td>
 *
 *</tr>
 * <tr>
 * 	<td> type </td>
 * 	<td> string </td>
 * 	<td> yes </th>
 *  	<td> &nbsp; </td>
 *      <td> Selects the LoggerType. To get a CSVLogger, this
 *           <b>must</b> be CSVLogger . </td>
 *</tr>
 *  <tr>
 * 	<td> datasource </td>
 * 	<td> string </td>
 * 	<td> yes </th>
 *  	<td> &nbsp; </td>
 *      <td> Name of the datasource. Must be eighter a name of a Inverter
 *           (Inverter Section)
 *      or a name of another DataFilter. </td>
 * </tr>
 * <tr>
 * 	<td> logfile  </td>
 * 	<td> string  </td>
 * 	<td> yes </th>
 *  	<td> &nbsp;  </td>
 *      <td> Defines the filename where solarpowerlog should write to.
 *      	See below for additional informations, </td>
 * </tr>
 * <tr>
 * 	<td> rotate  </td>
 * 	<td> bool  </td>
 * 	<td> &nbsp;</th>
 *  	<td> false  </td>
 *      <td> Create a new logfile on midnight. See the notes on logfile below.
 *      </td>
 * </tr>
 * <tr>
 * 	<td> data2log  </td>
 * 	<td> string or array  </td>
 * 	<td> &nbsp;</th>
 *  	<td> all  </td>
 *      <td> If the string reads "all", everything is logged. If an array is
 *      given, the data idenfied by the array will be loggged.
 *      See below for details.
 *      </td>
 * </tr>

 *
 * </table>

 * <b>logfile:</b>
 * This logger has the facility to start a new logfile at
 * midnight (only if the inverter is offline).
 * Solarpowerlog will, if rotate is enabled, create a new logfile.
 * To avoid name clashes, it will add the current date
 * (ISO 8601: YYYY-MM-DD ) to the filename.
 * To specify *where* the name should be placed, use "%s".
 * If %s is not given, it will be appended *at the end* of the filename ,
 * so must likely after the suffix.
 * For example
 * \code
 * 	logfile="Inverter_1_%s.csv"
 * \endcode
 * will create a logfile like "Inverter_1_2009-07-04.csv"
 *
 *
 * \todo TODO Allow logfile name, date feature, to specify by month, day, etc....
 *
 * <b> data2log:</b>
 * To specify which data should be logged, their ids have to be listed in a
 * array.
 *
 * The identifiers are usually described in the solarpower's inverter
 * documentation, in the documentation of the intermediate Datafilters or
 * by unsing the DumbDumper on the datasource.
 *
 * \note The identifiers are case sensitive!
 *
 * Array-Example:
 * \code
 *  data2log = [
 *  	"Current Grid Feeding Power",
 *  	"Energy produced today (kWh)",
 *  	"Energy produced this month (kWh)",
 *      "DC voltage in (V)"
 * ];
 * \endcode
 *
 * \note When selecting "all features" and new features are detected at runtime,
 * the CSV-header will be written again with the new data added to a new column.
 * If you want to avoid having ever-changing tables, please configure
 * all the data you want to see in the log file. Data which is not present all
 * the time will then still gets its placeholder in the output file.
 *
 * \todo  TODO Also a feature is planned, where "all" can be used, but data can be
 * removed with a "-" as first letter.
 *
 */

#ifndef CCSVOUTPUTFILTER_H_
#define CCSVOUTPUTFILTER_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_FILTER_CSVDUMP

#include <fstream>
#include <list>
#include "boost/date_time/local_time/local_time.hpp"

#include "DataFilters/interfaces/IDataFilter.h"


/** This logger writes the data to a CSV File
 *
 *
 *
 * Please see \ref DLCSV_Description for configuration, etc.
 *
 */


class CCSVOutputFilter : public IDataFilter
{
public:

	CCSVOutputFilter( const string &name, const string & configurationpath );

	virtual ~CCSVOutputFilter();

	virtual bool CheckConfig();

	virtual void  Update( const IObserverSubject *subject );

	virtual void ExecuteCommand( const ICommand *cmd );


private:

	IInverterBase *oursource;
	fstream file;

	/** Do the initialization of the module
	 *
	 * - subscribe to basic Capabilities
	 * - open logfile (and create filename)
	 * - schedule rotation of the filename at 00:00:05
	 * - schedule cyclic working
	 *
	 * Also will do the rotating of the logfile*/
	void DoINITCmd (const ICommand *);

	/** does the actual work:
	 *
	 * - maintain the list of columns to be written
	 * (the Capa map is sorted, but we need to make sure that
	 * we won't switch columns)
	 * - write the heade if necessary
	 * - check for data validty.
	 *  */
	void DoCYCLICmd(const ICommand *);

	enum Commands
	{
		CMD_INIT,
		CMD_CYCLIC,
		CMD_ROTATE ///<Rotate logfile
	};


	/** has the header been outputted to the file */
	bool headerwritten;

	/** is the data supplied by the inverter valid?*/
	bool datavalid;

	/** are there some updated capas? */
	bool capsupdated;

	/** list of Capabilities in the CSV */
	list<string> CSVCapas;


	// Helpers to shrink some functions...
	/** Check if any capas are now available which were not before
	 * (but should be tracked)
	 *
	 * The function will be called whenever the Capa-Updated event
	 * is set via this->Update()
	 *
	 * \return true, if a new Capability was detected and a new CSV Header
	 * should be generated. Otherwise false.
	 * */
	bool CMDCyclic_CheckCapas(void);

	/**  search the CSVCapas for a named capa
	 *
	 * \returns true, if in list, else false. */
	bool search_list(const string id) const;

	/// cache: last emitted string without timestamp
	std::string last_line;
};

#endif

#endif /* CCSVOUTPUTFILTER_H_ */
