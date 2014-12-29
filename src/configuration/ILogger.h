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

 This program is distributed in the hope that it will be useful, but
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
 *  - INFO: Verbatim information targeted to the user, showing details of
 *    the program flow, but not too much details.
 *    (showing when talking to a inverter, ...)
 *  - WARN:  This level indicates a minor problems, caused by external events.
 *    Usually some functions might be temporary not available.
 *  - ERROR: The program cannot function under this circumstances. The feature
 *    imposed will not be available until the reason is fixed and the program
 *    restarted.
 *    The program can usually continue to execute, but with the limitations.
 *  - FATAL: The program detected a problem which makes it impossible to
 *    continue. The program will usually call abort() after FATAL.
 *
 */

#ifndef ILOGGER_H_
#define ILOGGER_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <ostream>


#ifdef HAVE_LIBLOG4CXX
#include <log4cxx/logger.h>
#endif


#ifdef HAVE_LIBLOG4CXX

#define LOGFATAL(logger, message)  do \
	{\
		if ((logger).IsEnabled(ILogger::LL_FATAL)) { \
			std::stringstream ss;\
			ss << message;\
			logger << ss;\
		}\
	} while(0)

#define LOGERROR(logger, message)  do \
	{\
		if ((logger).IsEnabled(ILogger::LL_ERROR)) { \
			std::stringstream ss;\
			ss << message;\
			logger << ss;\
		}\
	} while(0)

#define LOGWARN(logger, message)   do \
	{\
		if ((logger).IsEnabled(ILogger::LL_WARN)) { \
			std::stringstream ss;\
			ss << message;\
			logger << ss;\
		}\
	} while(0)

#define LOGINFO(logger, message)   do \
	{\
		if ((logger).IsEnabled(ILogger::LL_INFO)) { \
			std::stringstream ss;\
			ss << message;\
			logger << ss;\
		}\
	} while(0)

#define LOGDEBUG(logger, message)   do \
	{\
		if ((logger).IsEnabled(ILogger::LL_DEBUG)) { \
			std::stringstream ss;\
			ss << message;\
			logger << ss;\
		}\
	} while(0)

#define LOGTRACE(logger, message)   do \
	{\
		if ((logger).IsEnabled(ILogger::LL_TRACE)) { \
			std::stringstream ss;\
			ss << message;\
			logger << ss;\
		}\
	} while(0)

#define LOGALL(logger, message)   do \
	{\
		if ((logger).IsEnabled(ILogger::LL_ALL)) { \
			std::stringstream ss;\
			ss << message;\
			logger << ss;\
		}\
	} while(0)

#else

#define LOGFATAL(logger, message) do { \
	cerr << message << endl;\
} while(0)

#define LOGERROR(logger, message) do { \
	cerr << message << endl;\
} while(0)

#define LOGWARN(logger, message)

#define LOGINFO(logger, message)

#define LOGDEBUG(logger, message)

#define LOGTRACE(logger, message)

#define LOGALL(logger, message)

#endif


#ifdef HAVE_LIBLOG4CXX

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

	enum level
	{
		LL_OFF = log4cxx::Level::OFF_INT,
		LL_FATAL = log4cxx::Level::FATAL_INT,
		LL_ERROR = log4cxx::Level::ERROR_INT,
		LL_WARN = log4cxx::Level::WARN_INT,
		LL_INFO = log4cxx::Level::INFO_INT,
		LL_DEBUG = log4cxx::Level::DEBUG_INT,
		LL_TRACE = log4cxx::Level::TRACE_INT,
		LL_ALL = log4cxx::Level::ALL_INT
	};

	/** Configure the logger with a name (to identify) ,
	 * the configuration string (for retrieving logger config)
	 * and a section (under what hierarchy to place the logger)
	 *
	 * \param name of the logger
	 * \param configurationpath where to retrieve the config
	 * \param sectin where to place the logger
	 *
	 */
	void Setup( const std::string &name, const std::string &configuration,
		const std::string& section );

	/** Adding a logger in a lower hierarchy level (below a parent object)
	 * by just specifying the parent.
	 *
	 * This logger will inheritate all settings by its parent, or, the XML
	 * file might configure it.
	 * */
	void Setup( const std::string &parent,
		const std::string &specialization );

	/** the default constructor set up logging with the root logger. */
	ILogger();

	virtual ~ILogger();

	/// Getter for loggername
	inline std::string getLoggername() const
	{
		return loggername_;
	}

	/// Modify the logger level (at runtime)
	void SetLoggerLevel(log4cxx::LevelPtr level);

	/** Check if a logging statement would go through and
	 * if so setup logging level.
	 *
	 * This function should be used before using the << operator, as
	 * this function will avoid calling all the ostream-operators,
	 * when indeed the logging level is below that one configured
	 * for the logger,
	 *
	 * The LOG_xxx macros will do that for you.
	 *
	 * Example:
	 * \code
	 * if (logger->IsEnabled(FATAL)) logger << "Fatal Error occured" <<endl;
	 * \endcode*/
	inline bool IsEnabled( int loglevel )
	{
		if (loglevel >= currentloggerlevel_) {
			currentlevel = loglevel;
			return true;
		} else
			return false;
	}

	/// set the next loglevel (the level which the app will log with the next time)
	/// usually this is not needed, as IsEnabled and LOG_xxx() do that for you
	///
	/// \param loglevel the level for subsequent logging.
	///
	/// \note: Please use LOG_xxx whenever possible.
	inline void SetLogLevel( int loglevel )
	{
		currentlevel = loglevel;
	}

	/// Log a string with the level.
	///
	/// \param loglevel level to log with
	/// \param log string to log
	/// \note when logging a static string, this might be more performant.
	/// than LOG_xxx.
	inline void Log( int loglevel, const std::string &log )
	{
		if (IsEnabled(loglevel))
			loggerptr_->log(log4cxx::Level::toLevel(loglevel),log);
	}

	/** provides the << operator for convenient logging (std::string version)
	 *
	 * \note the loglevel of the message has to be setup prior logging
	 *
	 * \note Use LOG_xxx() macros whenever possible.
	*/
	std::string & operator <<( std::string &os )
	{
		if (currentlevel >= currentloggerlevel_) {
			loggerptr_->forcedLog(log4cxx::Level::toLevel(
				currentlevel), os);
		}
		return os;
	}

	/** provides the << operator for convenient logging (stringstream version)
	 *
	 * \note the loglevel of the message has to be setup prior logging
	 *
	 * \note Use LOG_xxx() macros whenever possible.
	*/
	std::stringstream & operator <<( std::stringstream &os )
	{
		if (currentlevel >= currentloggerlevel_) {
			std::string s = os.str();
			loggerptr_->forcedLog(log4cxx::Level::toLevel(
				currentlevel), s);
		}
		return os;
	}

private:

	/// cache for the configuration string.
	std::string config_;

	/// cache for the loggers name.
	std::string loggername_;

	/// The logger of liblog4cxx...
	log4cxx::LoggerPtr loggerptr_;

	/// the last set log level from the application ("I want to log at this level next")
	int currentlevel;

	/// stores the logging level of the underlaying logger (cache)
	/// FIXME: Think about removing that one...
	/// should be queried from the logger, or at least updated when setting the level.
	int currentloggerlevel_;


};

#endif // HAVE_LIBLOG4CXX
#ifndef HAVE_LIBLOG4CXX
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

	enum level
	{
		LL_OFF = 0,
		LL_FATAL ,
		LL_ERROR ,
		LL_WARN ,
		LL_INFO ,
		LL_DEBUG ,
		LL_TRACE ,
		LL_ALL
	};
	/** Configure the logger with a name (to identify) ,
	 * the configuration string (for retrieving logger config)
	 * and a section (under what hierarchy to place the logger)
	 *
	 * \param name of the logger
	 * \param configurationpath where to retrieve the config
	 * \param sectin where to place the logger
	 *
	 */
	void Setup( const std::string &, const std::string &,
		const std::string&  ) { };

	/** Adding a logger in a lower hierarchy level (below a parent object)
	 * by just specifying the parent.
	 *
	 * This logger will inheritate all settings by its parent, or, the XML
	 * file might configure it.
	 * */
	void Setup( const std::string &,
			const std::string & ) {};

	/** the default constructor set up logging with the root logger. */
	ILogger() {};

	virtual ~ILogger() {};

	std::string getLoggername() const
	{
		return "";
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
	inline bool IsEnabled( int )
	{
		return false;
	}

	inline void SetLogLevel( int )
	{
	}

	inline void Log( int , std::string )
	{
	}

	std::string & operator <<( std::string &os )
	{
		return os;
	}

	std::stringstream & operator <<( std::stringstream &os )
	{
		return os;
	}

};
#endif // !HAVE_LIBLOG4CXX



#endif /* ILOGGER_H_ */
