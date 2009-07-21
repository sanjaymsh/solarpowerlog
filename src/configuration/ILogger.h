/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (ILogger.h), the license terms are:

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

/** \file ILogger.h
 *
 *  Created on: Jul 20, 2009
 *      Author: tobi
 */

#ifndef ILOGGER_H_
#define ILOGGER_H_

#include <string>
#include <ostream>

#include <log4cxx/logger.h>

/** Interface for logging services
 *
 * This class is the interface to the underlying logging class.
 * (planned: log4cxx)
 *
 * Loggers can be attached to every object, and log4cxx allows to structure
 * them into a hierarchy.
 *
 * This class is responsible to give every object access to its own logger,
 * and extract the logger's configuration out of the configuration file.
 * (allowing to configure the log for each component individually)
 *
 * The class is intended to use a composition or by inheritance.
 *
 */
class ILogger : public std::ostream
{
public:
	enum level
	{
		OFF = log4cxx::Level::OFF_INT,
		FATAL = log4cxx::Level::FATAL_INT,
		ERROR = log4cxx::Level::ERROR_INT,
		WARN = log4cxx::Level::WARN_INT,
		INFO = log4cxx::Level::INFO_INT,
		DEBUG = log4cxx::Level::DEBUG_INT
	};

	void Setup( const std::string &name, const std::string &configuration,
		const std::string& section );

	ILogger( ) {};
	virtual ~ILogger();

	/** Check if a logging statement would go through and
	 * if so setup logging level.
	 *
	 * This function should be used before using the << operator, as
	 * this function will avoid calling all the ostream-operators.
	 *
	 * Example:
	 * \code
	 * if (logger->LogIsEnabled(FATAL)) logger << "Fatal Error occured" <<endl;
	 * \endcode*/
	inline bool LogIsEnabled( int loglevel )
	{
		if (loglevel >= currentloggerlevel_) {
			currentlevel = loglevel;
			return true;
		} else
			return false;
	}

	inline void SetLogLevel( int loglevel )
	{
		currentlevel = loglevel;
	}

	inline void Log(int loglevel, std::string log) {
		if (LogIsEnabled(loglevel)) *this << log;
	}

	std::ostream & operator <<( std::ostream &os )
	{
		if (currentlevel >= currentloggerlevel_) {
			log4cxx::helpers::MessageBuffer oss_;
			loggerptr_->forcedLog(log4cxx::Level::toLevel(
				currentlevel), oss_.str(oss_ << os));
		}
		return os;
	}

private:
	std::string config_;
	std::string loggername_;
	log4cxx::LoggerPtr loggerptr_;
	int currentlevel;
	int currentloggerlevel_;
};

#endif /* ILOGGER_H_ */
