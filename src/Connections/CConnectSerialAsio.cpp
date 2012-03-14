/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

Copyright (C) 2010-2012 Tobias Frost

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

/** \file CConnectSerialAsio.cpp
 *
 * This interface class abstracts the Comms-Interface (IConnect) to the
 * boost::asio class
 *
 * Boost::asio can act either as a serial class (like RS233, Rs485)
 * or as TCP class. so copying would mean lots of duplicated code
 *
 *  Created on: May 16, 2010
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_COMMS_ASIOSERIAL

#define DEBUG_SERIALASIO

#include "interfaces/IConnect.h"
#include "Connections/CConnectSerialAsio.h"

#include <iostream>
#include <string>

#include "configuration/CConfigHelper.h"

#include <boost/asio/write.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

#include <boost/algorithm/string.hpp>

#include "interfaces/CMutexHelper.h"

#include <errno.h>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/date_time/posix_time/posix_time_config.hpp>
#include <boost/bind.hpp>

using namespace boost::posix_time;

using namespace std;
using namespace boost::asio;
using namespace boost;
using namespace libconfig;

struct asyncASIOCompletionHandler
{
	asyncASIOCompletionHandler(size_t *b, boost::system::error_code *ec)
	{
		bytes = b;
		this->ec = ec;
	}

	void operator()(const boost::system::error_code& e,
			std::size_t bytes_transferred)
	{
		*bytes = bytes_transferred;
		*ec = e;
	}

	// note, we need pointer as boost seems to make a copy of our handler...
	size_t *bytes;
	boost::system::error_code *ec;
};

/// Helping function for timeout and receive, will be called by boosts' asio.
/// this handler just will set the int store with the value value.
static void boosthelper_set_result(int* store, int value)
{
	if (store)
		*store = value;
}

CConnectSerialAsio::CConnectSerialAsio(const string &configurationname) :
	IConnect(configurationname)
{
	// Generate our own asio ioservice
	// TODO check if one central would do that too...
	ioservice = new io_service;
	port = new boost::asio::serial_port(*ioservice);
	sem_init(&cmdsemaphore, 0, 0);
}

CConnectSerialAsio::~CConnectSerialAsio()
{
	if (IsConnected()) {
		this->Disconnect(NULL);
	}

	if (port)
		delete port;
	if (ioservice)
		delete ioservice;
}

// TODO Extract to common base class (duplicate code here!!)
void CConnectSerialAsio::Connect(ICommand *callback)
{
	CAsyncCommand *commando = new CAsyncCommand(CAsyncCommand::CONNECT,
			callback);
	PushWork(commando);
}

/* Disconnect
 *
 * The disconnection is done by the async task.
 *
 * (If to be done synchronous, it is also dispatched to the worker thread and
 * directly waited for completion.)
 * */
void CConnectSerialAsio::Disconnect(ICommand *callback)
{
	// note: internally we still use the sync interface in the destructor!
	// to ensure that the port is closed when we tear down everything.

	sem_t semaphore;

	CAsyncCommand *commando = new CAsyncCommand(CAsyncCommand::DISCONNECT,
			callback);
	// if callback is NULL, fallback to synchronous operation.
	// (we will do the job asynchronous, but wait for completion here)
	if (!callback) {
		sem_init(&semaphore, 0, 0);
		commando->SetSemaphore(&semaphore);
	}

	PushWork(commando);

	if (!callback) {
		// wait for async job completion
		sem_wait(&semaphore);
		LOGTRACE(logger, "destroying CAsyncCommando " << commando );
		delete commando;
	}
}

void CConnectSerialAsio::Send(ICommand *callback)
{
	// note, callback must not be null, and also already prepared
	// e.g ICONN_TOKEN_SEND_STRING set.
	assert(callback);
	CAsyncCommand *commando = new CAsyncCommand(CAsyncCommand::SEND, callback);
	PushWork(commando);
}

/* Receive bytes from the stream -- asynced.
 *
 * As with all other methods, will be done by the worker thread, even if
 * synchronous operation is requested. In this case, we'll just wait for
 * completion.*/
void CConnectSerialAsio::Receive( ICommand *callback)
{
	CAsyncCommand *commando = new CAsyncCommand(CAsyncCommand::RECEIVE,
			callback);
	PushWork(commando);
}

bool CConnectSerialAsio::IsConnected(void)
{
	if (!port) return false;
	mutex.lock();
	bool ret = port->is_open();
	mutex.unlock();
	return ret;
}

bool CConnectSerialAsio::CheckConfig(void)
{
	string setting;
	bool fail = false;
	bool portsetting_parseerr = false;

	CConfigHelper cfghelper(ConfigurationPath);

	fail |= !cfghelper.CheckConfig("serial_serialportname",
			libconfig::Setting::TypeString, false);
	fail |= !cfghelper.CheckConfig("serial_baudrate",
			libconfig::Setting::TypeInt, false);
	fail |= !cfghelper.CheckConfig("serial_portparameters",
			libconfig::Setting::TypeString);
	fail |= !cfghelper.CheckConfig("serial_flowcontrol",
			libconfig::Setting::TypeString);
	fail |= !cfghelper.CheckConfig("serial_timeout",
			libconfig::Setting::TypeInt);
	fail |= !cfghelper.CheckConfig("serial_interbytetimeout",
			libconfig::Setting::TypeInt);

	cfghelper.GetConfig("", setting, std::string("none"));
	boost::algorithm::to_lower(setting);
	if (setting == "none") {
		flowctrl = boost::asio::serial_port_base::flow_control(
				boost::asio::serial_port_base::flow_control::none);
	} else if (setting == "hardware") {
		flowctrl = boost::asio::serial_port_base::flow_control(
				boost::asio::serial_port_base::flow_control::hardware);
	} else if (setting == "software") {
		flowctrl = boost::asio::serial_port_base::flow_control(
				boost::asio::serial_port_base::flow_control::software);
	} else {
		fail = true;
		LOGERROR(logger,"serial_flowcontrol must be \"none\", \"hardware\" or \"software\".");
	}

	cfghelper.GetConfig("serial_portparameters", setting, std::string("8N1"));

	if (setting.size() != 3) {
		LOGERROR(logger,"serial_portparameters: Must be exactly three "
				"characters");
	} else {

		if (setting[0] >= '5' || setting[0] <= '9') {
			this->characterlen = setting[0] - '0';
		} else {
			fail = true;
			portsetting_parseerr = true;
			LOGERROR(logger,"serial_portparameters: Number of bits per "
					"byte must be between 5 and 9" );
		}

		switch (setting[1])
		{
		case 'E':
		case 'e':
			parity = boost::asio::serial_port_base::parity(
					boost::asio::serial_port_base::parity::even);
			break;

		case 'O':
		case 'o':
			parity = boost::asio::serial_port_base::parity(
					boost::asio::serial_port_base::parity::odd);
			break;

		case 'N':
		case 'n':
			parity = boost::asio::serial_port_base::parity(
					boost::asio::serial_port_base::parity::none);
			break;

		default:
			portsetting_parseerr = true;
			LOGERROR(logger, "serial_portparameter: Invalid parity. Your "
					"choices are 'E'ven ,'O'dd or 'N'one");
			fail = true;
			break;
		}

		// If you are bored, you could implement 1.5 stop bits ;-)
		if (setting[2] == '1') {
			stopbits = boost::asio::serial_port_base::stop_bits(
					boost::asio::serial_port_base::stop_bits::one);
		} else if (setting[2] == '2') {
			stopbits = boost::asio::serial_port_base::stop_bits(
					boost::asio::serial_port_base::stop_bits::two);
		} else
			portsetting_parseerr = true;
		fail = true;
		LOGERROR(logger, "serial_portparameter: Invalid number of stop bits.");
	}

	if (portsetting_parseerr) {
		// print some explanations...
		LOGERROR(logger, "serial_portparameter must be of the format like"
				" \"8N1\" to specify symbol length(e.g  for example , parity "
				"and number of stopbits.)" );
	}


	if (!fail) {
		StartWorkerThread();
		return true;
	}

	return false;
}

void CConnectSerialAsio::_main(void)
{
	LOGTRACE(logger, "Starting helper thread");

	while (!IsTermRequested()) {
		int syscallret;

		// wait for work or signals.
		LOGTRACE(logger, "Waiting for work");
		syscallret = sem_wait(&cmdsemaphore);
		if (syscallret == 0) {
			// semaphore had work for us. process it.
			// safety check: really some work?
			mutex.lock();
			if (!cmds.empty()) {
				bool delete_cmd;
				CAsyncCommand *donow = cmds.front();
				// cache info if to delete the object later,
				// as later it might be already gone.
				delete_cmd = donow->IsAsynchronous();

				mutex.unlock();

				LOGTRACE(logger, "Received command " << donow << " with callback " << donow->callback );

				switch (donow->c)
				{
				case CAsyncCommand::CONNECT:
					if (HandleConnect(donow)) {
						LOGTRACE(logger, "Check command " << donow << " with callback " << donow->callback );
						mutex.lock();
						LOGTRACE(logger, "Front is " <<cmds.front() );
						cmds.pop_front();
						// check if we have to delete the object
						// or -- in case of sync operation --
						// the caller will do that for us.
						// the "sign" is, that donow->callback is non NULL
						// (as the object can be already gone, if
						// the sync command had already deleted it)
						if (delete_cmd) {
							LOGTRACE(logger, "Deleting " << donow);
							delete donow;
						}
						mutex.unlock();
					}
					break;

				case CAsyncCommand::DISCONNECT:
					if (HandleDisConnect(donow)) {
						mutex.lock();
						cmds.pop_front();
						if (delete_cmd) {
							LOGTRACE(logger, "Deleting " << donow);
							delete donow;
						}
						mutex.unlock();
					}
					break;

				case CAsyncCommand::RECEIVE:
					if (HandleReceive(donow)) {
						mutex.lock();
						cmds.pop_front();
						if (delete_cmd) {
							LOGTRACE(logger, "Deleting " << donow);
							delete donow;
						}
						mutex.unlock();
					}
					break;

				case CAsyncCommand::SEND:
				{
					if (HandleSend(donow)) {
						mutex.lock();
						cmds.pop_front();
						if (delete_cmd) {
							LOGTRACE(logger, "Deleting " << donow);
							delete donow;
						}
						mutex.unlock();
					}
				}
					break;

				default:
				{
					LOGFATAL(logger, "Unknown command received.");
					abort();
					break;
				}
				}

			} else {
				mutex.unlock();
			}

		}
	}

	IConnect::_main();
}

bool CConnectSerialAsio::PushWork(CAsyncCommand *cmd)
{
	LOGTRACE(logger, "Pushing command " << cmd << " with callback " << cmd->callback );
	mutex.lock();
	cmds.push_back(cmd);
	mutex.unlock();
	sem_post(&cmdsemaphore);
	workerthread.interrupt();
	return true;
}

bool CConnectSerialAsio::HandleConnect(CAsyncCommand *cmd)
{
	// most likely, the inverter's check config did not call CheckConfig
	// of this connection method.
	assert(baudrate);

	string portname;
	boost::system::error_code ec;

	// if connected, ignore the commmand, pretend success.
	if (IsConnected()) {
		cmd->callback->addData(ICMD_ERRNO, 0);
		cmd->HandleCompletion();
		return true;
	}

	CConfigHelper cfghelper(ConfigurationPath);

	cfghelper.GetConfig("serial_serialportname", portname);

	// we need first to open the port, before applying the settings to it.
	ec = port->open(portname, ec);

	if (ec) {
		// retrieve error code out of ec object. (With luck this works... After testing we'll know more)
		// Boost doc won't tell if it is negative, so we better make sure
		// (most likely it is, as it is done by an enum)
		int eint = ec.value();
		if (eint > 0)
			eint = -eint;
		cmd->callback->addData(ICMD_ERRNO, eint);
		if (!ec.message().empty()) {
			cmd->callback->addData(ICMD_ERRNO_STR, ec.message());
		}
		cmd->HandleCompletion();
		return true;
	}

	port->set_option(boost::asio::serial_port_base::baud_rate(baudrate));
	port->set_option(
			boost::asio::serial_port_base::character_size(characterlen));
	port->set_option(stopbits);
	port->set_option(parity);
	port->set_option(flowctrl);

	LOGDEBUG(logger, "Opened " << portname );
	// Signal success.

	// Saftey check -- could also be an assert...
	if (!port->is_open()) {
		cmd->callback->addData(ICMD_ERRNO, -EIO);
		cmd->callback->addData(ICMD_ERRNO_STR, std::string(
				"Error: Open succeeded but port not open. WTF?"));
		cmd->HandleCompletion();
		return true;
	}

	cmd->callback->addData(ICMD_ERRNO, 0);
	cmd->HandleCompletion();
	return true;

}

bool CConnectSerialAsio::HandleDisConnect(CAsyncCommand *cmd)
{
	boost::system::error_code ec, ec2;
	std::string message;
	int error = 0;

	if (!IsConnected()) {
		cmd->callback->addData(ICMD_ERRNO, 0);
		cmd->HandleCompletion();
		return true;
	}

	ec = port->cancel(ec);
	ec2 = port->close(ec2);

	if (ec) {
		error = -EIO;
		message = ec.message();
	}

	if (ec2) {
		error = -EIO;
		if (!message.empty())
			message = message + " ";
		message = message + ec2.message();
	}

	cmd->callback->addData(ICMD_ERRNO, error);
	if (!message.empty()) {
		cmd->callback->addData(ICMD_ERRNO_STR, ec.message());
	}

	cmd->HandleCompletion();
	return true;
}

/** Handle Receive -- asynchronous read from the asio socket with timeout.
 *
 * Strategy:
 * -- get timeout config from configuration
 * -- spawn timer with timeoput setting
 * -- run timer in background (async)
 * -- setup async read operation of one byte (to detect incoming comms)
 * -- check if we got byte or timeout
 * -- if got the byte, try to read all available bytes
 * -- if got the timer, cancel socket read and return error.
 *
 * HandleReceive expects a std::string in CAsyncCommand->auxData.
 */
bool CConnectSerialAsio::HandleReceive(CAsyncCommand *cmd)
{
	boost::system::error_code ec, handlerec;

	volatile int result_timer = 0;
	size_t bytes;
	unsigned long timeout;
	char buf[2];
	struct asyncASIOCompletionHandler read_handler(&bytes, &handlerec);
	// timeout setup
	ioservice->reset();

	try {
		timeout = boost::any_cast<unsigned long>(cmd->callback->findData(
				ICONN_TOKEN_TIMEOUT));
	} catch (std::invalid_argument &e) {
		CConfigHelper cfghelper(ConfigurationPath);
		cfghelper.GetConfig("serial_timeout", timeout, TCP_ASIO_DEFAULT_TIMEOUT);
	} catch (boost::bad_any_cast &e) {
		LOGDEBUG(logger, "Unexpected exception in HandleReceive: Bad cast" << e.what());
		timeout = TCP_ASIO_DEFAULT_TIMEOUT;
	}

	deadline_timer timer(*(this->ioservice));
	boost::posix_time::time_duration td = boost::posix_time::millisec(timeout);
	timer.expires_from_now(td);
	timer.async_wait(boost::bind(&boosthelper_set_result, (int*) &result_timer,
			1));

	// socket preparation
	//  async_read. However, boost:asio seems not to allow auto-buffers,
	// so we will just read one byte and when this is available, we'll
	// check for if there are some others left
	port->async_read_some(boost::asio::buffer(&buf, 1), read_handler);

	size_t num = ioservice->run_one(ec);

	// ioservice error or timeout
	if (num == 0 || result_timer) {
		timer.cancel(ec);
		port->cancel(ec);
		LOGTRACE(logger,"Async read timeout");
		cmd->callback->addData(ICMD_ERRNO, -ETIMEDOUT);
		cmd->HandleCompletion();
		ioservice->poll();
		return true;
	}

	timer.cancel();
	ioservice->poll(ec);

	if (*read_handler.ec) {
		if (*read_handler.ec != boost::asio::error::eof) {
			LOGDEBUG(logger,"Async read failed with ec=" << *read_handler.ec
					<< " msg="<< read_handler.ec->message());
			cmd->callback->addData(ICMD_ERRNO, -EIO);
			cmd->callback->addData(ICMD_ERRNO_STR, read_handler.ec->message());

		} else {
			cmd->callback->addData(ICMD_ERRNO, -ENOTCONN);
			LOGTRACE(logger, "Received eof on socket read");
		}
		cmd->HandleCompletion();
		return true;
	}

	/* now one byte is read -- we apply the byte-timeout here to read the
	 * remaining bytes. */

	try {
		timeout = boost::any_cast<unsigned long>(cmd->callback->findData(
				ICONN_TOKEN_INTERBYTETIMEOUT));
	} catch (std::invalid_argument &e) {
		CConfigHelper cfghelper(ConfigurationPath);
		cfghelper.GetConfig("serial_interbytetimeout", timeout, 0UL);
		if (timeout == 0) {
			// default interbyte timeout is 10 times the time for one byte.
			// (we allow the inaccurary and assume 10 bits per byte, which is
			// valid for 8N1)
			// however, we ensure a minimum time of 40 ms.
			// (which still might be tough as our OS might idle around for even longer)
			timeout = (1000 * 10 * 10) / baudrate;
			if (timeout <= TCP_ASIO_DEFAULT_INTERBYTETIMEOUT) timeout = TCP_ASIO_DEFAULT_INTERBYTETIMEOUT;
		}

	} catch (boost::bad_any_cast &e) {
		LOGDEBUG(logger, "Unexpected exception in HandleReceive: Bad cast" << e.what());
		timeout = TCP_ASIO_DEFAULT_TIMEOUT;
	}

	std::string received;
	received[0] = buf[0];

	while (1) {
		// prepare timer
		bytes = 0;
		result_timer = 0;
		ec.clear();
		handlerec.clear();
		td = boost::posix_time::millisec(timeout);
		timer.expires_from_now(td);
		timer.async_wait(boost::bind(&boosthelper_set_result, (int*) &result_timer,
				1));

		// read a byte
		port->async_read_some(boost::asio::buffer(&buf, 1), read_handler);

		size_t num = ioservice->run_one(ec);

		// ioservice error or timeout
		if (num == 0 || result_timer) {
			timer.cancel(ec);
			port->cancel(ec);
			ioservice->poll();
			break;
		}

		timer.cancel();
		ioservice->poll(ec);

		// asio async read completed -- check for read error
		if (*read_handler.ec) {
			if (*read_handler.ec != boost::asio::error::eof) {
				// read error occured, which is not timeout.
				LOGDEBUG(logger,"Async read failed with ec=" << *read_handler.ec
						<< " msg="<< read_handler.ec->message());
				cmd->callback->addData(ICMD_ERRNO, -EIO);
				cmd->callback->addData(ICMD_ERRNO_STR, read_handler.ec->message());
				cmd->HandleCompletion();
				return true;
			} else {
				// other end closed connection -- treat that as timeout
				break;
			}
		}

		// append read byte to string.
		received.push_back(buf[0]);
	}

	// we got at least one byte -- assemble answer.
	LOGTRACE(logger,"Serial read " << received.length() << " bytes" );

	cmd->callback->addData(ICONN_TOKEN_RECEIVE_STRING, received);
	cmd->callback->addData(ICMD_ERRNO, 0);
	cmd->HandleCompletion();

	return true;
}

/** handles async sending */
bool CConnectSerialAsio::HandleSend(CAsyncCommand *cmd)
{

	boost::system::error_code ec, handlerec;
	volatile int result_timer = 0;
	size_t bytes;
	unsigned long timeout;
	struct asyncASIOCompletionHandler write_handler(&bytes, &handlerec);
	// timeout setup
	ioservice->reset();
	std::string s;

	try {
		s = boost::any_cast<std::string>(cmd->callback->findData(
				ICONN_TOKEN_SEND_STRING));
	}
#ifdef DEBUG_SERIALASIO
	catch (std::invalid_argument &e) {
		LOGDEBUG(logger, "BUG: required " << ICONN_TOKEN_SEND_STRING << " argument not set");

	} catch (boost::bad_any_cast &e) {
		LOGDEBUG(logger, "Unexpected exception in HandleSend: Bad cast" << e.what());
	}
#else
	catch (...);
#endif

	try {
		timeout = boost::any_cast<unsigned long>(cmd->callback->findData(
				ICONN_TOKEN_TIMEOUT));
	}
#ifdef DEBUG_SERIALASIO
	catch (std::invalid_argument &e) {
		CConfigHelper cfghelper(ConfigurationPath);
		cfghelper.GetConfig("tcptimeout", timeout, 3000UL);
	} catch (boost::bad_any_cast &e) {
		LOGDEBUG(logger, "Unexpected exception in HandleSend: Bad cast" << e.what());
		timeout = TCP_ASIO_DEFAULT_TIMEOUT;
	}
#else
	catch (...) {
		CConfigHelper cfghelper(ConfigurationPath);
		cfghelper.GetConfig("tcptimeout", timeout, 3000UL);
	}
#endif

	deadline_timer timer(*(this->ioservice));
	boost::posix_time::time_duration td = boost::posix_time::millisec(timeout);
	timer.expires_from_now(td);
	timer.async_wait(boost::bind(&boosthelper_set_result, (int*) &result_timer,
			1));

	// socket preparation
	//  async_write the whole std::string
	boost::asio::async_write(*port, boost::asio::buffer(s), write_handler);

	// run one of the scheduled services
	size_t num = ioservice->run_one(ec);

	// ioservice error or timeout
	if (num == 0 || result_timer) {
		timer.cancel(ec);
		port->cancel(ec);
		LOGTRACE(logger,"Async write timeout");
		cmd->callback->addData(ICMD_ERRNO, -ETIMEDOUT);
		cmd->HandleCompletion();
		ioservice->poll();
		return true;
	}

	// cancel the timer, and catch the completion handler
	timer.cancel();
	ioservice->poll(ec);

	if (*write_handler.ec) {
		if (*write_handler.ec != boost::asio::error::eof) {
			LOGDEBUG(logger,"Async write failed with ec=" << *write_handler.ec
					<< " msg="<< write_handler.ec->message());
			cmd->callback->addData(ICMD_ERRNO, -EIO);
			cmd->callback->addData(ICMD_ERRNO_STR, write_handler.ec->message());
		} else {
			cmd->callback->addData(ICMD_ERRNO, -ENOTCONN);
			LOGTRACE(logger, "Received eof on socket write");
		}
		cmd->HandleCompletion();
		return true;
	}

	if (s.length() != *write_handler.bytes) {
		LOGDEBUG(logger,"Sent "
				<< *write_handler.bytes << " but expected "<< s.length() );
		cmd->callback->addData(ICMD_ERRNO, -EIO);
		cmd->HandleCompletion();
		return true;
	}

	cmd->callback->addData(ICMD_ERRNO, 0);
	cmd->HandleCompletion();
	return true;
}

#endif /* HAVE_COMMS_ASIOSERIAL */
