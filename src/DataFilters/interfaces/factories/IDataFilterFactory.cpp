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

/** \file IDataFilterFactory.cpp
 *
 *  \date Jun 1, 2009
 *  \author Tobias Frost (coldtobi)
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "DataFilters/interfaces/factories/IDataFilterFactory.h"

#ifdef HAVE_FILTER_DUMBDUMP
#include "DataFilters/CDumpOutputFilter.h"
#endif

#ifdef HAVE_FILTER_CSVDUMP
#include "DataFilters/CCSVOutputFilter.h"
#endif

#ifdef HAVE_FILTER_HTMLWRITER
#include "DataFilters/HTMLWriter/CHTMLWriter.h"
#endif

#ifdef HAVE_FILTER_DBWRITER
#include "DataFilters/DBWriter/CDBWriterFilter.h"
#endif

#include "configuration/Registry.h"
#include "configuration/CConfigHelper.h"


IDataFilter *IDataFilterFactory::Factory(const string & configurationpath)
{

	string type, name;
	CConfigHelper cfghlp(configurationpath);

	cfghlp.GetConfig("type", type);
	cfghlp.GetConfig("name",name);

#ifdef HAVE_FILTER_DUMBDUMP
	if (type == "DumbDumper") {
		return new CDumpOutputFilter(name, configurationpath);
	}
#endif

#ifdef HAVE_FILTER_CSVDUMP
	if (type == "CVSWriter") {
		return new CCSVOutputFilter(name, configurationpath);
	}
#endif

#ifdef 	HAVE_FILTER_HTMLWRITER
	if (type == "HTMLWriter") {
		return new CHTMLWriter(name, configurationpath);
	}
#endif

#ifdef HAVE_FILTER_DBWRITER
	if ( type == "DBWriter") {
	    return new CDBWriterFilter(name, configurationpath);
	}
#endif

	return NULL;
}
