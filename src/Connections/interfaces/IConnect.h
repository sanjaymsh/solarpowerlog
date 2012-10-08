/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2009-2012 Tobias Frost

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
/// or retrieve a configuration value.
/// unit is ms.
#define ICONN_TOKEN_TIMEOUT "ICON_TIMEOUT"

/** Timeout specifier -- this (optional) parameter sets the timeout between two
 *  bytes. It is only honoured by connection methods that "act" serial like.
 *  One prominent example would be comms over serial line.
 *  (note: normally this is not needed to be specified -- it can be set by
 *  the configuration file or calculated at runtime out of the baudrate. )
 *  Unit of the value by the token are ms.   */
#define ICONN_TOKEN_INTERBYTETIMEOUT "ICON_INTERBYTE_TIMEOUT"

/** Interface for all communication classes
 *
 * This Interface is the API for all concrete communication methods.
 * The class is abstract, so it cannot be instantiated by itself.
 *
 * Anyway, it is intended, that the class is only created by the IConnectFactory.
 *
 * <b>Asynchronous operations </b>
 *
 * The commands are dispatched using the ICommand interface. In the ICommand
 * objects the data is placed, identification is possible using the token
 *
 * The result is placed into the ICommand-data, also using the token system.
 *
 * For example, result codes are placed in the token "ICMD_ERRNO" with the data
 * as "integer", using the "errno" conventions:
 * Negative numbers indicates an error.
 * Please see the documentation of the methods for expected error codes.
 * (Storage is done with the boost::any classes, see the ICommand interface)
 *
 * <b> ICommand ownership </b>
 * As usual, ICommands that are submitted to this class, will be owned by this class.
 * So do not delete them, as they will be automatically deleted by the WorkScheduler.
 *
 * \note If async operations would be overkill, because the result is
 * immediately known, one can also implement the async commands synchronously,
 * as long as it uses the ICommand for notification of the result.
 *
 * This can be done by directly setting the result in the provided ICommand and
 * call Registry::GetMainScheduler()->ScheduleWork(ICommand) afterward.
 * CConnectDummy does this in its Dispatch_Error() routine.
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

	/// Setup the Logger.
	virtual void SetupLogger(const string& parentlogger, const string &spec =
			"Comms");

	virtual ~IConnect();

	/** Connects to the target, establish communication link.
     *
     * The target and the settings are retrieved out of the configuration.
     *
     * Connect asynchronous and use the ICommand to tell the result in ICMD_ERRNO
     *
     * \note If asynchronous operations would be overkill, because the result is
     * immediately known, one can also implement the asynchronsous operations
     * as synchronous ones as long as it uses the ICommand as notification
     * for the result.
     */
	virtual void Connect(ICommand *callback) = 0;

	/** Tear down the connection.
	 *
	 * Disconnect async and use the ICommand to tell the result.
	 *
	 * The result will be placed in the supplied ICommand's data field.
	 *
	 * The value will be usually EIO, as one has to ask himself: What can go wrong here?
	 *
	 * \note Try hard to get the comm into a known state, where connect will
	 * be able to recover, or reconnection might be futile as recovery strategy.
	 *
	 */
	virtual void Disconnect(ICommand *callback) = 0;

	/** Asynchronous send interface
	 * The data needs now to be embedded as data into the ICommand using the
	 * token ICONN_TOKEN_SEND_STRING.
	 *
	 * As usual, results are passed using the ICommand supplied.
	*/
	virtual void Send(ICommand *cmd) = 0;

	/** Receive data from connection and place it into a std::string
	 *
	 * Try to receive data from the other end and place everything readed
	 * into the supplied std::string.
	 *
	 * \param cmd ICommand to be used for async notification.
	 *
	 * The result is presented in the returned ICommand, using those TOKENs:
	 *
	 * ICMD_ERRNO  -- for errors: If you receive a value <0 -- error happened.
	 * 	the type in this boost:any is integer.
	 * ICMD_ERRNO_STR -- optional for an human readable error message.
	 * 	However, it is recommended to set this token.
	 * 	the boost:any type is std::string
	 * ICONN_TOKEN_RECEIVE_STRING -- received data from communication.
	 * 	the boost:any type is std::string
	 *
	 * Regarding ICMD_ERRNO, this errorno are defined and should be used / evaluated
	 * 	EIO	I/O Error on the comms. Reason unknown or something
	 * 		unexpected happended. (one should close and reopen the connection)
	 *
	 *	ETIMEDOUT Read request timed out: No bytes received during
	 *		configured timeout.
	 *
	 *	ENOTCONN  Connection went down, e.g. eof received.
	 */
	virtual void Receive(ICommand *cmd) = 0;

	/// Check the configuration for validity. Return false on config errors.
	/// (program will abort then!)
	virtual bool CheckConfig(void) = 0;

	/// Check at runtime if the communication class supports the "Accept"
	/// method.
	/// \returns true if Accept() works, false if not.

	virtual bool CanAccept(void) {
	    return false;
	}

	/// Check if we believe the connection is still active
	/// Note: if the concrete implementation cannot tell,
	/// it should always return true, as the default implementation does.
	/// (the inverter class has to do some kind of timeout-handling anyway)
	virtual bool IsConnected(void)
	{
		return true;
	}

	/// Accept an inbound connection to be used for the communication.
	/// (if supported by the underlying transport mechanism.)
	/// \param cmd callback object to be used after completion.
	/// \note cmd must be provided (non-NULL)
	/// \returns false if Accept is not supported.
	/// \sa CanAccept
	/// \warning If you use Accept() and Connect() on the same object,
	/// the behavior is undefined.
	virtual bool Accept(ICommand */*cmd*/) {
	    return false;
	}

	/// Abort all pending commands
	/// (Try to) abort everything pending, like waiting for all I/O and clear
	/// all other (queued) commands.
	/// Answer all unfinished command with an error, preferable ECANCELED.
	/// returns true if succeeded, false if not supported by the connection
	/// object
	virtual bool AbortAll() = 0;

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
