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
 *
 *  \section LoggingLevels Loggging Levels
 *
 *  Here some guideline to choose the right logging level to keep consistent
 *  throughout the program.
 *
 *  - TRACE: Very verbatim (debug purpose) infos, like protocol, telegramms,
 *    low level data .... This level should be used to gather every information
 *    that might be needed to debug problems... Milestones during execution
 *    also counts to this. The goal is to find out whats happening if debugging
 *    some rare problems.
 *  - DEBUG: "Regular" Debug infos, like tracepoints, unusual program flow
 *    detection, etc. Detected problems that are likely a programming problem....
 *  - INFO: Verbatim informations targeted to the user, showing details of
 *    the program flow, but not too much details.
 *    (showing when talking to a inverter, ...)
 *  - WARN:  This level indicates a minor problems, caused by external events.
 *    Usually some functions might be temporary not available.
 *  - ERROR: The program cannot function under this circumstances. The feature
 *    imposed will not be available until the reason is fixed and the programm
 *    restarted.
 *    The program can usually continue to execute, but with the limitations.
 *  - FATAL: The program detected a problem which makes it impossible to
 *    continue. The program will usually call abort() after FATAL.
 *
 */

#ifndef ILOGGER_H_
#define ILOGGER_H_

#include <string>
#include <ostream>

#ifdef HAVE_LIBLOG4CXX
#include <log4cxx/logger.h>
#endif


#ifdef HAVE_LIBLOG4CXX

#define LOG_FATAL(logger, message)  do \
	{\
		if (logger.IsEnabled(ILogger::FATAL)) { \
			std::stringstream ss;\
			ss << message;\
			logger << ss;\
		}\
	} while(0)

#define LOG_ERROR(logger, message)  do \
	{\
		if (logger.IsEnabled(ILogger::ERROR)) { \
			std::stringstream ss;\
			ss << message;\
			logger << ss;\
		}\
	} while(0)

#define LOG_WARN(logger, message)   do \
	{\
		if (logger.IsEnabled(ILogger::WARN)) { \
			std::stringstream ss;\
			ss << message;\
			logger << ss;\
		}\
	} while(0)

#define LOG_INFO(logger, message)   do \
	{\
		if (logger.IsEnabled(ILogger::INFO)) { \
			std::stringstream ss;\
			ss << message;\
			logger << ss;\
		}\
	} while(0)

#define LOG_DEBUG(logger, message)   do \
	{\
		if (logger.IsEnabled(ILogger::DEBUG)) { \
			std::stringstream ss;\
			ss << message;\
			logger << ss;\
		}\
	} while(0)

#define LOG_TRACE(logger, message)   do \
	{\
		if (logger.IsEnabled(ILogger::TRACE)) { \
			std::stringstream ss;\
			ss << message;\
			logger << ss;\
		}\
	} while(0)

#define LOG_ALL(logger, message)   do \
	{\
		if (logger.IsEnabled(ILogger::ALL)) { \
			std::stringstream ss;\
			ss << message;\
			logger << ss;\
		}\
	} while(0)

#else

#define LOG_FATAL(logger, message) do { \
	cerr << message << endl;\
} while(0)

#define LOG_ERROR(logger, message) do { \
	cerr << message << endl;\
} while(0)

#define LOG_WARN(logger, message)

#define LOG_INFO(logger, message)

#define LOG_DEBUG(logger, message)

#define LOG_TRACE(logger, message)

#define LOG_ALL(logger, message)

#endif

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
class ILogger /*: public std::ostream*/
{
public:


#ifdef HAVE_LIBLOG4CXX
	enum level
	{
		OFF = log4cxx::Level::OFF_INT,
		FATAL = log4cxx::Level::FATAL_INT,
		ERROR = log4cxx::Level::ERROR_INT,
		WARN = log4cxx::Level::WARN_INT,
		INFO = log4cxx::Level::INFO_INT,
		DEBUG = log4cxx::Level::DEBUG_INT,
		TRACE = log4cxx::Level::TRACE_INT,
		ALL = log4cxx::Level::ALL_INT
	};
#endif

	/** Configure the logger with a name (to identify) ,
	 * the configuration string (for retrieving logger config)
	 * and a section (under what hierarchy to place the logger)
	 *
	 * \param name of the logger
	 * \param configurationpath where to retrieve the config
	 * \param sectin where to place the logger
	 *
	 */
#ifdef HAVE_LIBLOG4CXX
	void Setup( const std::string &name, const std::string &configuration,
		const std::string& section );
#else
	void Setup( const std::string &name, const std::string &configuration,
		const std::string& section ) {};
#endif


	/** Adding a logger in a lower hierarchy level (below a parent object)
	 * by just specifing the parent.
	 *
	 * This logger will inheritate all settings by its parent, or, the XML
	 * file might configure it.
	 * */
#ifdef HAVE_LIBLOG4CXX
	void Setup( const std::string &parent,
		const std::string &specialization );
#else
	void Setup( const std::string &parent,
			const std::string &specialization ) {};
#endif

	/** the default constructor set up logging with the root logger. */
#ifdef HAVE_LIBLOG4CXX
	ILogger();
#else
	ILogger() {};
#endif

#if defined HAVE_LIBLOG4CXX
	virtual ~ILogger();
#else
	virtual ~ILogger() {};
#endif

	std::string getLoggername() const
	{
#if defined HAVE_LIBLOG4CXX
		return loggername_;
#else
		return "";
#endif
	}

	/** Check if a logging statement would go through and
	 * if so setup logging level.
	 *
	 * This function should be used before using the << operator, as
	 * this function will avoid calling all the ostream-operators.
	 *
	 * Example:
	 * \code
	 * if (logger->IsEnabled(FATAL)) logger << "Fatal Error occured" <<endl;
	 * \endcode*/
	inline bool IsEnabled( int loglevel )
	{
#if defined HAVE_LIBLOG4CXX
		if (loglevel >= currentloggerlevel_) {
			currentlevel = loglevel;
			return true;
		} else
			return false;
#else
		return false;
#endif
	}

	inline void SetLogLevel( int loglevel )
	{
#if defined HAVE_LIBLOG4CXX
		currentlevel = loglevel;
#endif
	}

	inline void Log( int loglevel, std::string log )
	{
#if defined HAVE_LIBLOG4CXX
		if (IsEnabled(loglevel))
			loggerptr_->log(log4cxx::Level::toLevel(loglevel),log);
#endif
	}


	std::string & operator <<( std::string &os )
	{
#if defined HAVE_LIBLOG4CXX
		if (currentlevel >= currentloggerlevel_) {
			loggerptr_->forcedLog(log4cxx::Level::toLevel(
				currentlevel), os);
		}
#endif
		return os;
	}

	std::stringstream & operator <<( std::stringstream &os )
	{
#if defined HAVE_LIBLOG4CXX
		if (currentlevel >= currentloggerlevel_) {
			std::string s = os.str();
			loggerptr_->forcedLog(log4cxx::Level::toLevel(
				currentlevel), s);
		}
#endif
		return os;
	}

#if defined HAVE_LIBLOG4CXX
private:

	std::string config_;
	std::string loggername_;
	log4cxx::LoggerPtr loggerptr_;

	int currentlevel;
	int currentloggerlevel_;
#endif

};

#endif /* ILOGGER_H_ */
