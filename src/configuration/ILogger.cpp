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


/** Construct a logger object with default settings.
 *
 * When using this constructor, the logger will attach to the root
 * logger by default. This can be overridden by a subsequent call Setup() */
ILogger::ILogger()
{
	// if not overridden by setup, always log to the root logger.
	loggerptr_ = log4cxx::Logger::getRootLogger();
	currentloggerlevel_ = loggerptr_->getLevel()->toInt();
}

/** Setup logger in the logger, attaching to a parent.
 *
 * The parent logger and the specialization forms the path of the new logger
 * for easier identification of the source of the message. The new logger is
 * put one level deeper than its parent.
 *
 * For example, Inverter_1 adds a Comm which specalizsation of "Comms_TCP_ASIO",
 * the resulting logger is Inveter_1.Comms_TCP_ASIO
 *
 * The logging level will be deducted from the parent, (default
 * from log4cxx).
 *
 * \param parent logger to attach from. Must not be empty.
 * \param specialization for the logger. (the name of the new one)
 */
void ILogger::Setup( const std::string & parent,
	const std::string & specialization )
{
	loggername_ = parent + "." + specialization;
	log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger(loggername_));
	log4cxx::LevelPtr ptr=logger->getEffectiveLevel();
	currentloggerlevel_ = ptr->toInt();
	loggerptr_ = logger;
}


/** Setup a logger. Do not associate to a parent
 *
 * (of course, the root logger is always parent)
 *
 * This variant creates a logger with the path <section>.<name>
 * <section> is the name of the section in the configuration file,
 * <name> the name of the object,
 *
 * The Settings for the logger are extracted from the configuration
 * file and with this precedence:
 * - Specification in XML file (liblog4cxx config)
 * - <section>.<name> enry dbglevel in the solarpowerlog.conf
 * - application.dbglevel in the solarpowerlog.conf
 * - "ERROR" if nothing is given,
 *
 * \param name of the new logger
 * \param configuration path where to obtain te objects config.
 *
*/
void ILogger::Setup( const string & name, const string & configuration,
	const string& section )
{
	string level;
	config_ = configuration;

	loggername_ = section + "." + name;

	log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger(loggername_));
	loggerptr_ = logger;

	// check if the logger has magically already a level.
	// if so, it must be from XML.
	log4cxx::LevelPtr ptr=logger->getLevel();
	if (!ptr) {

		CConfigHelper global("application");
		global.GetConfig("dbglevel", level, (std::string) "ERROR");

		CConfigHelper hlp(configuration);
		hlp.GetConfig("dbglevel", level);

		logger->setLevel(log4cxx::Level::toLevel(level));
	}

	currentloggerlevel_ = logger->getLevel()->toInt();
}

void ILogger::SetLoggerLevel(log4cxx::LevelPtr level) {
	loggerptr_->setLevel(level);
	currentloggerlevel_ = level->toInt();
}


ILogger::~ILogger()
{
	// TODO Auto-generated destructor stub
}

#endif
