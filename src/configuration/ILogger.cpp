/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (ILogger.cpp), the license terms are:

 You can redistribute it and/or modify it under the terms of the GNU
 General Public License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This programm is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/** \file ILogger.cpp
 *
 *  Created on: Jul 20, 2009
 *      Author: tobi
 */

#include "ILogger.h"

#include <log4cxx/logger.h>
#include "configuration/CConfigHelper.h"
#include <iostream>

using namespace std;

void ILogger::Setup( const string & name, const string & configuration,
	const string& section )
{
	if (loggerptr_) {
		LOG4CXX_DEBUG(loggerptr_, "BUG: Logger" << section << '.' <<
			name << " already instanciated!");
		return;
	}

	string level;
	// name_ = name;
	config_ = configuration;
	// section_ = section;

	loggername_ = section + "." + name;

	log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger(loggername_));
	loggerptr_ = logger;

	CConfigHelper global("application");
	global.GetConfig("dbglevel", level, (std::string) "ERROR");

	CConfigHelper hlp(configuration);
	hlp.GetConfig("dbglevel", level);

	cout << "Effective debug level " << level << endl;

	logger->setLevel(log4cxx::Level::toLevel(level));
	currentloggerlevel_ = logger->getLevel()->toInt();
	LOG4CXX_DEBUG(logger, "logger for " << configuration << "."
		<< name << "created.");
}

ILogger::~ILogger()
{
	// TODO Auto-generated destructor stub
}
