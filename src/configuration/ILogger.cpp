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

#include "config.h"

#include "configuration/ILogger.h"

#ifdef HAVE_LIBLOG4CXX

#include "configuration/CConfigHelper.h"
#include <iostream>

using namespace std;

ILogger::ILogger()
{
	// if not overridden by setup, always log to the root logger.
	loggerptr_ = log4cxx::Logger::getRootLogger();
}

void ILogger::Setup( const std::string & parent,
	const std::string & specialization )
{
	loggername_ = parent + "." + specialization;
	log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger(loggername_));
	loggerptr_ = logger;

}

void ILogger::Setup( const string & name, const string & configuration,
	const string& section )
{
	string level;
	config_ = configuration;

	loggername_ = section + "." + name;

	log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger(loggername_));
	loggerptr_ = logger;

#warning missing: if XML configuring is choosen, the XML should be able to override the \
	settings here. But this code will still take priority.

	CConfigHelper global("application");
	global.GetConfig("dbglevel", level, (std::string) "ERROR");

	CConfigHelper hlp(configuration);
	hlp.GetConfig("dbglevel", level);

	logger->setLevel(log4cxx::Level::toLevel(level));
	currentloggerlevel_ = logger->getLevel()->toInt();
	LOG4CXX_DEBUG(logger, "Logger for " << configuration << "."
		<< name << " created.");
}

ILogger::~ILogger()
{
	// TODO Auto-generated destructor stub
}

#endif
