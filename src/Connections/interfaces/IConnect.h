/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (IConnect.h), the license terms are:

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

/** \file IConnect.h
 *
 *  Created on: May 16, 2009
 *      Author: tobi
 */

#ifndef ICONNECT_H_
#define ICONNECT_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>

#include "configuration/ILogger.h"
#include <boost/thread.hpp>
#include "patterns/ICommand.h"
#include <errno.h>

using namespace std;

// USED ICOMMAND TOKENS
/// Receive-result of the Transaction (std::string)
/// might be not present in case of error.
#define ICONN_TOKEN_RECEIVE_STRING	"ICON_RECEIVE_STRING"

/// (private token) Send this string over the connection
/// This is used to communicate to the worker thread what it should send.
#define ICONN_TOKEN_SEND_STRING	"ICON_SEND_STRING"

/// Timeout modifier -- with this optional parameter the timeout parameter
/// can be overridden from the config for the current operation.
/// This allows fine-grade timeouts for any operation
/// Note: If not specified, the implementation will
/// either use a default value
/// or retrieve a configuration value
#define ICONN_TOKEN_TIMEOUT "ICON_TIMEOUT"

/** Timeout specifier -- this (optional) parameter sets the timeout between two
 *  bytes. It is only honoured by connection methods that "act" serial like.
 *  One prominent example would be comms over serial line.
 *  (note: normally this is not needed to be specified -- it can be set by
 *  the configuration file or calculated at runtime out of the baudrate. )
 *  Unit of the value by the token are ms.   */
#define ICONN_TOKEN_INTERBYTETIMEOUT "ICON_INTERBYTE_TIMEOUT"

#warning docs outdated!
/** Interface for all communication classes
 *
 * This Interface is the API for all concrete communication methods.
 * The class is abstract, so it cannot be instantiated by itself.
 *
 * Anyway, it is intended, that the class is only created by the IConnectFactory.
 *
 * <b>Synchronous and asynchronous operations </b>
 *
 * \warning the synchronous interface is depreciated.
 *
 * (Preliminary, as currently in development) FIXME Update docs when ready!
 * When the asynch operations are implemented, synchronous operations should
 * be depreciated, as the might infer with the timings of other inverters.
 *
 * The IConnect methods can be used synchronous and asyncronous,( if the
 * communication classes support this. But the plans are to make these all async-capable)
 *
 * The customers of the IConnect classes can choose if they want to use the
 * async or sync methods by specifying optional ICommand arguments. If they are
 * obmitted (and therefore defaulting to NULL pointers) the command will use
 * synchronous methods, so returning the result directly (boolean).
 * The booleans are used because of how solarpowerlog developed
 *
 * If the asynchronous commands are used, the result is placed into the ICommand-data
 * void pointer, which has to be casted to an int. Here "errno" conventions
 * are used. Negative numbers tell that an error occured, zero means success.
 * Please see the documentation of the methods for expected error codes.
 *
 * \note A convention developed during implemention of the first async classes,
 * is, when programming methods, only return errors if the
 * object cannot perform its function. So, for example, if the call requires
 * the object in a specific state, only return an error if this state cannot be
 * easily reache. For example, if Connect()-ing, don't return an error when already
 * connected, just return silently success.
 *
 * <b> ICommand ownership </b>
 * As usual, ICommands that are submitted to this class, will be owned by this class.
 * So do not delete them, as they will be automatically deleted by the WorkScheduler.
 *
 * \note If Async operations would be overkill, because the result is
 * immediatly known, one can also implement the async ops as synchronous
 * as long as it uses the async notification methods.
 *
 */
class IConnect
{
public:
	/// Constructor.
	///
	/// The constructor gets the configuration path to be used to extract
	/// its configuration.
	///
	IConnect(const string &configurationname);

	/// Setupp the Logger.
	virtual void SetupLogger(const string& parentlogger, const string &spec =
			"Comms");

	/// Desctructor
	virtual ~IConnect();

	/// Connect to something
	/// NOTE: Needed to be overriden! ALWAYS Open in a NON_BLOCK way, or implement a worker thread

	/** Connect to the target, so comms can happen.
	 *
	 * The target and the settings are retrieved out of the configuration.
	 *
	 * SYNCHRONOUNS OPERATION:
	 * Connect and return if it has succeeded. May block, but should be limited
	 * to some seconds.
	 *
	 * ASYNCHRONOUS OPERATION:
	 * Connect async and use the ICommand to tell the result.
	 *
	 * \note If a connection is already up, return "true" or success (async)
	 *
	 * \note On asynch operation, the return can be ignored.
	 *
	 * \note If Async operations would be overkill, because the result is
	 * immediatly known, one can also implement the async ops as synchronous
	 * as long as it uses the async notification methods.
	 */
	virtual bool Connect(ICommand *callback) = 0;

	/** Tear down the connection.
	 *
	 * SYNCHRONOUNS OPERATION:
	 * Just Disconnect. If this could fail, return false on failure, else true.
	 *
	 * ASYNCHRONOUS OPERATION:
	 * Disconnect async and use the ICommand to tell the result.
	 *
	 * \note If a connection is already up, return "true" or success (async)
	 *
	 * \returns true on successful reading (synchronous operation only),
	 * will always return true on asynch operation, as error notification
	 * solely to be done by the supplied cmd.
	 *
	 * ASYNCHRNOUNS ERROR CODES.
	 * The result will be placed in the supplied ICommand's data field.
	 *
	 * The value will be usually EIO, as one hs to ask himself: What can go wrong here?
	 *
	 * \note Try hard to get the comm into a known state, where connect will
	 * be able to recover, or reconnection might be futile as recovery strategy.
	 *
	 */
	virtual bool Disconnect(ICommand *callback) = 0;

#warning depreciate synchronous interface

	/// Send a array of characters (can be used as binary transport, too)
	///
	/// \warning not all classes might understand binary transport. Check
	/// the concrete implmentation for details!
	/// \param tosend what to send
	/// \param len how many bytes
	/// \returns true on success, false on error.
	virtual bool Send(const char *tosend, unsigned int len, ICommand *callback) = 0;

	/// Send a string
	/// \note Standard implementation only wraps to above Send-binray.
	///
	/// \param tosend std::string to send
	/// \returns true on success, false on error.
	/// Override for better performance!
	virtual bool Send(const string& tosend, ICommand *callback)
	{
		return Send(tosend.c_str(), tosend.length(), callback);
	}

	/** Receive data from connection and place it into a std::string
	 *
	 * Try to receive data from the other end and place everything readed
	 * into the supplied std::string.
	 *
	 * \param wheretoplace object for data storage.
	 * \param cmd ICommand to be used for async notification.
	 *
	 * \note on async operation, where to place has to be valid until the
	 * notification has been received.
	 *
	 * \note When the read operation has failed, the contents of the
	 * string-object is undefined.
	 * (The class might alter it, even when the read fails.
	 *
	 * \returns true on successful reading (synchronous operation only),
	 * will always return true on asynch operation, as error notification
	 * solely to be done by the supplied cmd.
	 *
	 * ASYNCHRONOUS ERRORS:
	 * The implementation might use errors out of error.h to specify the cause
	 * of the error.
	 *
	 * These error values are defined in their meaning:
	 *
	 * 	EIO	I/O Error on the comms. Reason unknown or something
	 * 		unexpected happended. (one should close and reopen the connection)
	 *
	 *	ETIMEDOUT Read request timed out: No bytes received during
	 *		configured timeout.
	 *
	 *	ENOTCONN  Connection went down, e.g. eof received.
	 */
	// virtual bool Receive( string &wheretoplace, ICommand *cmd = NULL ) = 0;

	virtual bool Receive(ICommand *cmd) = 0;

public:

	/// Check the configuration for validty. Retrurn false on config errors.
	/// (program will abort then!)
	virtual bool CheckConfig(void) = 0;

	/// Check if we believe the connection is still active
	/// Note: if the concrete implementation cannot tell,
	/// it should always return true, as the default implementaion does.
	/// (the inverter class has to do some kind of timeout-handling anyway)
	virtual bool IsConnected(void)
	{
		return true;
	}

protected:
	/// Storage for the Configuration Path to extract settings.
	string ConfigurationPath;

	/// Associated logger.
	/// \note: By default, it will attached to the root logger. If unwanted,
	/// use the SetupLogger() call. (As IInverterBase derived classes do)
	ILogger logger;

	/// ASYNC OPERATION

	/// Thread
	boost::thread workerthread;

	/// Mutex to protect data
	boost::recursive_mutex mutex;

	/// function of the thread.
	/// \note: if overridden, the overriding function has to call this one
	/// right before exiting!
	virtual void _main(void)
	{
		mutex.lock();
		_thread_is_running = false;
		mutex.unlock();
	}

	/// Start the Worker thread.
	virtual void StartWorkerThread(void);

	/// Check if termination of the worker thread has been requested
	virtual bool IsTermRequested(void);

	/// Check if thread is running
	///
	/// \note this does not check if the thread has crashed, only if it has not
	/// self-terminated or never started..
	virtual bool IsThreadRunning(void);

private:
	/// bool to check if the thread is started
	bool _thread_is_running;

	/// bool to request termination
	bool _thread_term_request;

};

#endif /* ICONNECT_H_ */
