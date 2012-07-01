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


/** \file CConnectionTCPAsio.cpp
 *
 *  Created on: May 21, 2009
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_COMMS_ASIOTCPIO

#define DEBUG_TCPASIO

#include "interfaces/IConnect.h"
#include "Connections/CConnectTCPAsio.h"

#include <iostream>
#include <string>

#include "configuration/CConfigHelper.h"

#include <boost/asio/write.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

#include <boost/scoped_ptr.hpp>

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
	asyncASIOCompletionHandler( size_t *b, boost::system::error_code *ec )
	{
		bytes = b;
		this->ec = ec;
	}

	void operator()( const boost::system::error_code& e,
			std::size_t bytes_transferred )
	{
		*bytes = bytes_transferred;
		*ec = e;
	}

	// note, we need pointer as boost seems to make a copy of our handler...
	size_t *bytes;
	boost::system::error_code *ec;
};

CConnectTCPAsio::CConnectTCPAsio( const string &configurationname ) :
	IConnect(configurationname)
{
	// Generate our own asio ioservice
	// TODO check if one central would do that too...
    configured_as_server = false;
	ioservice = new io_service;
	sockt = new ip::tcp::socket(*ioservice);
	sem_init(&cmdsemaphore, 0, 0);
}

CConnectTCPAsio::~CConnectTCPAsio()
{
	if (IsConnected()) {
		Disconnect(NULL);
	}

	if (sockt)
		delete sockt;
	if (ioservice)
		delete ioservice;

}

// ASYNCED!!
void CConnectTCPAsio::Connect( ICommand *callback )
{
	assert(callback);
	CAsyncCommand *commando = new CAsyncCommand(CAsyncCommand::CONNECT, callback);
	PushWork(commando);
}

/* Disconnect
 *
 * The disconnection is done by the async task.
 *
 * (If to be done synchronous, it is also dispatched to the worker thread and
 * directly waited for completion.)
 * */
void CConnectTCPAsio::Disconnect( ICommand *callback )
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

void  CConnectTCPAsio::Send( ICommand *callback)
{
	assert(callback);
	CAsyncCommand *commando = new CAsyncCommand(CAsyncCommand::SEND, callback);
	PushWork(commando);
}

/* Receive bytes from the stream -- asynced.
 *
 * As with all other methods, will be done by the worker thread
*/
void CConnectTCPAsio::Receive( ICommand *callback )
{
	// RECEIVE async Command:
	// auxdata: pointer to std::string, where to place received data

	assert(callback);
	CAsyncCommand *commando = new CAsyncCommand(CAsyncCommand::RECEIVE, callback);
	PushWork(commando);
}

bool CConnectTCPAsio::IsConnected( void )
{
	if (!this->sockt) return false;
	mutex.lock();
	bool ret = sockt->is_open();
	mutex.unlock();
	return ret;
}

bool CConnectTCPAsio::CheckConfig( void )
{
	string setting;
	bool fail = false;
	CConfigHelper cfghelper(ConfigurationPath);

	if (cfghelper.GetConfig("tcpmode",setting)) {
	    if (setting == "server") {
	        fail |= !cfghelper.CheckConfig("tcpadr", libconfig::Setting::TypeString,true);
	        fail |= !cfghelper.CheckConfig("tcpport", libconfig::Setting::TypeInt);
	        if (!fail) this->configured_as_server = true;
	    }
	}

	if (!configured_as_server) {
        fail |= !cfghelper.CheckConfig("tcpadr", libconfig::Setting::TypeString);
        fail |= !cfghelper.CheckConfig("tcpport", libconfig::Setting::TypeString);
        fail |= !cfghelper.CheckConfig("tcptimeout", libconfig::Setting::TypeInt,
                false);
	}

	if (!fail) {
		StartWorkerThread();
		return true;
	}

	return false;
}

void CConnectTCPAsio::_main( void )
{
	LOGTRACE(logger, "Starting helper thread");

	while (!IsTermRequested()) {
		int syscallret;

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

				switch (donow->c) {
				case CAsyncCommand::CONNECT:
					if (HandleConnect(donow)) {
						mutex.lock();
						cmds.pop_front();
						// check if we have to delete the object
						// or -- in case of sync operation --
						// the caller will do that for us.
						// the "sign" is, that donow->callback is non NULL
						// (as the object can be already gone, if
						// the sync command had already deleted it)
						if (delete_cmd) {
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
							delete donow;
						}
						mutex.unlock();
					}
					break;

				case CAsyncCommand::SEND:
				{
					if(HandleSend(donow)) {
						mutex.lock();
						cmds.pop_front();
						if (delete_cmd) {
							delete donow;
						}
						mutex.unlock();
					}
				}
				break;

                case CAsyncCommand::ACCEPT:
                {
                    if (HandleAccept(donow)) {
                        mutex.lock();
                        cmds.pop_front();
                        if (delete_cmd) {
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

bool CConnectTCPAsio::PushWork( CAsyncCommand *cmd )
{
	mutex.lock();
	cmds.push_back(cmd);
	mutex.unlock();
	sem_post(&cmdsemaphore);
	workerthread.interrupt();
	return true;
}

bool CConnectTCPAsio::HandleConnect( CAsyncCommand *cmd )
{
	string strhost, port;
	unsigned long timeout = -1;

	// if connected, ignore the commmand, pretend success.
	if (IsConnected()) {
		cmd->callback->addData(ICMD_ERRNO, 0);
		cmd->HandleCompletion();
		return true;
	}

	// fail, if we are configured for inbound connections.
    if (configured_as_server) {
        cmd->callback->addData(ICMD_ERRNO,-EPERM);
        cmd->HandleCompletion();
        return true;
    }

	CConfigHelper cfghelper(ConfigurationPath);

#warning timeouts should be only configured in the calling objects! \
	Otherwise it is hard to differenicate between commands! \
	So depreciate tcptimeout and configure this in the inverter class!
#warning rework: Should be only needed from the configuration, as the \
	calling object needs not be aware of these issues (should be transparent)

	try {
		timeout = boost::any_cast<long>(cmd->callback->findData(
				ICONN_TOKEN_TIMEOUT));
	} catch (std::invalid_argument &e) {
		cfghelper.GetConfig("tcptimeout", timeout, TCP_ASIO_DEFAULT_TIMEOUT);
	} catch (boost::bad_any_cast &e) {
		LOGDEBUG(logger,
				"BUG: Handling Connect: Bad cast for " << ICONN_TOKEN_TIMEOUT);
		timeout = TCP_ASIO_DEFAULT_TIMEOUT;
	}

	cfghelper.GetConfig("tcpadr", strhost);
	cfghelper.GetConfig("tcpport", port);

	boost::system::error_code ec;
	ip::tcp::resolver resolver(*ioservice);
	ip::tcp::resolver::query query(strhost.c_str(), port);

	// returns on error a default constructed iterator ...
	ip::tcp::resolver::iterator iter = resolver.resolve(query, ec);
	ip::tcp::resolver::iterator end; // ... which is a "End marker" itself.

#warning TODO timeouts not yet considered -- change to async_connect
	while (iter != end) {
		ip::tcp::endpoint endpoint = *iter++;
		LOGDEBUG(logger, "Connecting to " << endpoint );
		sockt->connect(endpoint, ec);
		if (!ec)
			break;
	}

	// preset name, but only needed if we gonna log on these levels.
	if (logger.IsEnabled(ILogger::LL_ERROR) || logger.IsEnabled(ILogger::LL_DEBUG))
		cfghelper.GetConfig("name", strhost);

	if (ec) {
		cmd->callback->addData(ICMD_ERRNO, -ECONNREFUSED);
		if (!ec.message().empty())
			cmd->callback->addData(ICMD_ERRNO_STR, ec.message());
		cmd->HandleCompletion();
		return true;
	}

	LOGDEBUG(logger, "Connected to " << strhost );
	// Signal success.
	cmd->callback->addData(ICMD_ERRNO, 0);
	cmd->HandleCompletion();
	return true;
}

bool CConnectTCPAsio::HandleDisConnect( CAsyncCommand *cmd )
{
	boost::system::error_code ec, ec2;
	std::string message;
	int error = 0;

	if (!IsConnected()) {
		cmd->callback->addData(ICMD_ERRNO, 0);
		cmd->HandleCompletion();
		return true;
	}

	// according boost:asio documentation, one should call shutdown before
	// close
	// (http://www.boost.org/doc/libs/1_36_0/doc/html/boost_asio/reference/basic_stream_socket/close/overload2.html)
	sockt->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

	sockt->close(ec2);

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

/// Helping function for timeout and receive, will be called by boost::asio.
/// this handler just will set the int store with the value value.
static void boosthelper_set_result( int* store, int value )
{
	if (store)
		*store = value;
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
bool CConnectTCPAsio::HandleReceive( CAsyncCommand *cmd )
{
	boost::system::error_code ec, handlerec;
	volatile int result_timer = 0;
	size_t bytes = 0;
	unsigned long timeout = 0;
	char buf[2];
	struct asyncASIOCompletionHandler read_handler(&bytes, &handlerec);
	// timeout setup
	ioservice->reset();

	try {
		timeout = boost::any_cast<unsigned long>(
				cmd->callback->findData(ICONN_TOKEN_TIMEOUT));
	} catch (std::invalid_argument &e) {
		CConfigHelper cfghelper(ConfigurationPath);
		cfghelper.GetConfig("tcptimeout", timeout, TCP_ASIO_DEFAULT_TIMEOUT);
	} catch (boost::bad_any_cast &e) {
		LOGDEBUG(logger,
				"Unexpected exception in HandleReceive: Bad cast" << e.what());
		timeout = TCP_ASIO_DEFAULT_TIMEOUT;
	}

	deadline_timer timer(*(this->ioservice));
	boost::posix_time::time_duration td = boost::posix_time::millisec(timeout);
	timer.expires_from_now(td);
	timer.async_wait(
			boost::bind(&boosthelper_set_result, (int*) &result_timer, 1));

	// socket preparation
	// async_read: boost:asio seems not to allow auto-buffers,
	// so we will just read one byte and when this is available, we'll
	// check for if there are some others left

	// seen when connecting to localhost: ioservice returns, with ec = eof,
	// but still connected. So if retry to async_read_some as long as timeout
	// not expired. (but we will limit to a few times in case of a real eof and
	// a long timeout)
	size_t num = 0;
	int maxsleep = 5; // we limit the hack to 5 times.
	do {
		bool sleep = false;
		if (!sleep) {
			boost::posix_time::time_duration td2 = boost::posix_time::millisec(
					25);
			deadline_timer timer(*ioservice, td2);
			try {
				timer.wait();
				maxsleep--;
			} catch (...) {
			}
		}
		sleep = true;
		sockt->async_read_some(boost::asio::buffer(&buf, 1), read_handler);
		num = ioservice->run_one(ec);
	} while (maxsleep != 0 && !result_timer && 0 == *(read_handler.bytes));

    if (num == 0) {
		LOGTRACE(logger, "HandleReceive: IO Service error: " << ec.message());
	}
	// ioservice error or timeout
	if (num == 0 || result_timer) {
		timer.cancel(ec);
		sockt->cancel(ec);
		LOGTRACE(logger,"Async read timeout");
		cmd->callback->addData(ICMD_ERRNO_STR, std::string("Read timeout"));
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

	if (1 != *read_handler.bytes) {
		LOGDEBUG(logger,"Received "
				<< *read_handler.bytes << " but expected only 1 byte");
		cmd->callback->addData(ICMD_ERRNO, -EIO);
		cmd->HandleCompletion();
		return true;
	}

	size_t avail = sockt->available();
	size_t tmp;
	size_t numrecvd = 1;
	std::string receivestr;

	LOGTRACE(logger, "There are " << avail << " bytes ready to read");
	char recved[avail + 2];
	recved[0] = buf[0];
	while (avail > 0) {
		tmp = sockt->read_some(asio::buffer(&recved[numrecvd], avail), ec);
		LOGTRACE(logger, "Read " << tmp << " of these");
		avail -= tmp;
		numrecvd += tmp;
		// check if error occured.
		if (ec) {
			int error;

			switch (ec.value()) {
			case boost::asio::error::eof:
				error = -ENOTCONN;
				break;
			default:
				error = -EIO;
				break;
			}

			// give everything we got before the error.
			receivestr.assign(recved, numrecvd);
			cmd->callback->addData(ICONN_TOKEN_RECEIVE_STRING, receivestr);
			cmd->callback->addData(ICMD_ERRNO_STR, ec.message());
			cmd->callback->addData(ICMD_ERRNO, error);
			cmd->HandleCompletion();
			return true;
		}
	}

	receivestr.assign(recved, numrecvd);
	cmd->callback->addData(ICONN_TOKEN_RECEIVE_STRING, receivestr);
	cmd->callback->addData(ICMD_ERRNO, 0);
	cmd->HandleCompletion();
	return true;
}

bool CConnectTCPAsio::Accept(ICommand *callback)
{
    assert(callback); // does not support sync operation!
    CAsyncCommand *commando = new CAsyncCommand(CAsyncCommand::ACCEPT,
            callback);
    PushWork(commando);
    return true;
}

/** handles async sending */
bool CConnectTCPAsio::HandleSend( CAsyncCommand *cmd ) {

	boost::system::error_code ec, handlerec;
	volatile int result_timer = 0;
	size_t bytes = 0;
	unsigned long timeout;
	struct asyncASIOCompletionHandler write_handler(&bytes, &handlerec);
	// timeout setup
	ioservice->reset();
	std::string s;

	try {
		s = boost::any_cast<std::string>(cmd->callback->findData(ICONN_TOKEN_SEND_STRING));
	}
#ifdef DEBUG_TCPASIO
	catch (std::invalid_argument &e) {
		LOGDEBUG(logger, "BUG: required " << ICONN_TOKEN_SEND_STRING << " argument not set");

	}
	catch (boost::bad_any_cast &e)
	{
		LOGDEBUG(logger, "Unexpected exception in HandleSend: Bad cast" << e.what());
	}
#else
	catch (...);
#endif

	try {
		timeout = boost::any_cast<unsigned long>(cmd->callback->findData(
				ICONN_TOKEN_TIMEOUT));
	}
#ifdef DEBUG_TCPASIO
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
	boost::asio::async_write(*sockt, boost::asio::buffer(s), write_handler);

	// run one of the scheduled services
	size_t num = ioservice->run_one(ec);

	// ioservice error or timeout
	if (num == 0 || result_timer) {
		timer.cancel(ec);
		sockt->cancel(ec);
		LOGTRACE(logger,"Async write timeout");
		cmd->callback->addData(ICMD_ERRNO, -ETIMEDOUT);
		cmd->HandleCompletion();
		ioservice->poll();
		return true;
	}

	// cancel the timer, and catch the completion handler
	timer.cancel();
	ioservice->poll(ec);

	LOGTRACE(logger,"Sent " << *write_handler.bytes << "Bytes");

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

// Server mode. Listen to incoming connections.
bool CConnectTCPAsio::HandleAccept(CAsyncCommand *cmd)
{
    int port;
    std::string ipadr;
	ioservice->reset();
    // Do not accept if already connected.
    if (IsConnected()) {
        cmd->callback->addData(ICMD_ERRNO, -EBUSY);
        cmd->HandleCompletion();
        return true;
    }

    // Fail if this is not configured as a server.
    if (!configured_as_server) {
        cmd->callback->addData(ICMD_ERRNO,-EPERM);
        cmd->HandleCompletion();
        return true;
    }

    CConfigHelper cfghelper(ConfigurationPath);

    cfghelper.GetConfig("tcpadr", ipadr,std::string("any"));
    cfghelper.GetConfig("tcpport", port);

    boost::scoped_ptr<ip::tcp::endpoint> endpoint;

    if (ipadr == "any") {
        endpoint.reset(new ip::tcp::endpoint(ip::tcp::v4(),port));
    } else if ( ipadr == "any_v6")
    {
       endpoint.reset(new ip::tcp::endpoint(ip::tcp::v6(),port));
    } else {
        ip::address adr = ip::address::from_string(ipadr);
       endpoint.reset(new ip::tcp::endpoint(adr,port));
    }

    boost::system::error_code ec;
    LOGDEBUG(logger,"Waiting for inbound connection on " << ipadr << ":" << port);

    try {
        ip::tcp::acceptor acceptor(*ioservice, *endpoint);
        acceptor.listen();
        acceptor.accept(*sockt, ec);
    } catch (boost::system::system_error &e) {
        LOGDEBUG(logger, "Boost: exception received while accepting.");
        ec = e.code();
    }
    if (ec) {
        int eval = -ec.value();
        if (!eval) { eval = -1; }
        cmd->callback->addData(ICMD_ERRNO, eval);
        if (!ec.message().empty()) {
            cmd->callback->addData(ICMD_ERRNO_STR, ec.message());
        }
        cmd->HandleCompletion();
        LOGDEBUG( logger,
                "Connection failed. Error " << eval <<
                "(" << ec.message() << ")");
        return true;
    }

    LOGTRACE(logger, "Connected.");
    cmd->callback->addData(ICMD_ERRNO, 0);
    cmd->HandleCompletion();
    return true;
}

#endif /* HAVE_COMMS_ASIOTCPIO */
