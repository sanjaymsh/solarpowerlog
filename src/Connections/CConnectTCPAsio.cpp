/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (ConnectionTCP.cpp), the license terms are:

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

/** \file ConnectionTCPBoost.cpp
 *
 *  Created on: May 21, 2009
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#include "interfaces/IConnect.h"
#include "Connections/CConnectTCPAsio.h"

#include <iostream>
#include <string>

#include "configuration/CConfigHelper.h"

#include <boost/asio/write.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

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

struct asyncReadHandler
{
	asyncReadHandler( size_t *b, boost::system::error_code *ec )
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
	ioservice = new io_service;
	sockt = new ip::tcp::socket(*ioservice);

	StartWorkerThread();
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
bool CConnectTCPAsio::Connect( ICommand *callback )
{
	sem_t semaphore;

	asyncCommand *commando = new asyncCommand(asyncCommand::CONNECT,
		callback);

	// if callback is NUll, fallback to synchronous operation.
	if (!callback) {
		sem_init(&semaphore, 0, 0);
		commando->SetSemaphore(&semaphore);
	}

	PushWork(commando);

	if (!callback) {
		sem_wait(&semaphore);
		LOG_TRACE(logger, "destroying asyncCommando" << commando );
		delete commando;
		return IsConnected();
	}

	return true;
}

// ASYNCED!!
bool CConnectTCPAsio::Disconnect( ICommand *callback )
{
	sem_t semaphore;

	asyncCommand *commando = new asyncCommand(asyncCommand::DISCONNECT,
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
		LOG_TRACE(logger, "destroying asyncCommando" << commando );
		delete commando;
	}

	return true;
}

// Send kept SYNC for the moment
bool CConnectTCPAsio::Send( const char *tosend, unsigned int len )
{
	size_t written;
	written = asio::write(*sockt, asio::buffer(tosend, len));
	return (written == len);
}

// Send kept SYNC for the moment
bool CConnectTCPAsio::Send( const string & tosend )
{
	return Send(tosend.c_str(), tosend.length());
}

bool CConnectTCPAsio::Receive( string & wheretoplace, ICommand *callback )
{
	// RECEIVE async Command:
	// auxdata: pointer to std::string, where to place received data

	sem_t semaphore;
	bool ret;

	asyncCommand *commando = new asyncCommand(asyncCommand::RECEIVE,
		callback);
	commando->SetAuxData((void *) &wheretoplace);

	if (!callback) {
		sem_init(&semaphore, 0, 0);
		commando->SetSemaphore(&semaphore);
	}

	PushWork(commando);

	if (!callback) {
		// sync: wait for async job completion and check result.
		sem_wait(&semaphore);
		if (commando->GetResult() < 0) {
			ret = false;
		} else {
			ret = true;
		}
		LOG_TRACE(logger, "destroying asyncCommando" << commando );
		delete commando;
		return ret;
	}

	// async job accepted!
	return true;
}

// ASYNCED
bool CConnectTCPAsio::IsConnected( void )
{
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

	fail
		|= !cfghelper.CheckConfig("tcpadr",
			libconfig::Setting::TypeString);
	fail |= !cfghelper.CheckConfig("tcpport",
		libconfig::Setting::TypeString);
	fail |= !cfghelper.CheckConfig("tcptimeout",
		libconfig::Setting::TypeInt, false);

	return !fail;
}

void CConnectTCPAsio::_main( void )
{
	LOG_TRACE(logger, "Starting helper thread");
	mutex.lock();
	sem_init(&cmdsemaphore, 0, 0);
	mutex.unlock();

	while (!IsTermRequested()) {
		int syscallret;
		ICommand *cmd;

		// wait for work or signals.
		// FIXME Test this if this works ;-)
		LOG_TRACE(logger, "Waiting for work");
		syscallret = sem_wait(&cmdsemaphore);
		if (syscallret == 0) {
			// semaphore had work for us. process it.
			// safety check: really some work?
			mutex.lock();
			if (!cmds.empty()) {
				asyncCommand *donow = cmds.front();
				// cache donow->callback, as it could be gone
				// later.
				cmd = donow->callback;
				mutex.unlock();

				LOG_TRACE(logger, "Received command " << donow << " with callback " << donow->callback );

				switch (donow->c) {
				case asyncCommand::CONNECT:
					if (HandleConnect(donow)) {
						LOG_TRACE(logger, "Check command " << donow << " with callback " << donow->callback );
						mutex.lock();
						LOG_TRACE(logger, "Front is " <<cmds.front() );
						cmds.pop_front();
						// check if we have to delete the object
						// or -- in case of sync operation --
						// the caller will do that for us.
						// the "sign" is, that donow->callback is non NULL
						// (as the object can be already gone, if
						// the sync command had already deleted it)
						if (cmd)
							delete donow;
						mutex.unlock();
					}
					break;

				case asyncCommand::DISCONNECT:
					if (HandleDisConnect(donow)) {
						mutex.lock();
						cmds.pop_front();
						if (cmd)
							delete donow;
						mutex.unlock();
					}
					break;

				case asyncCommand::RECEIVE:
					if (HandleReceive(donow)) {
						mutex.lock();
						cmds.pop_front();
						if (cmd)
							delete donow;
						mutex.unlock();
					}
					break;
				}
			} else {
				mutex.unlock();
			}

		}
	}

	IConnect::_main();
}

bool CConnectTCPAsio::PushWork( asyncCommand *cmd )
{
	LOG_TRACE(logger, "Pushing command " << cmd << " with callback " << cmd->callback );
	mutex.lock();
	cmds.push_back(cmd);
	mutex.unlock();
	sem_post(&cmdsemaphore);
	workerthread.interrupt();
	return true;
}

bool CConnectTCPAsio::HandleConnect( asyncCommand *cmd )
{
	string strhost, port;
	unsigned long timeout;

	CConfigHelper cfghelper(ConfigurationPath);
	cfghelper.GetConfig("tcpadr", strhost);
	cfghelper.GetConfig("tcpport", port);
	cfghelper.GetConfig("tcptimeout", timeout, 3000UL);

	// if connected, ignore the commmand, pretend success.
	if (IsConnected()) {
		cmd->HandleCompletion((void*) 0);
		return true;
	}

	boost::system::error_code ec;
	ip::tcp::resolver resolver(*ioservice);
	ip::tcp::resolver::query query(strhost.c_str(), port);
	ip::tcp::resolver::iterator iter = resolver.resolve(query);
	ip::tcp::resolver::iterator end; // End marker.

	// TODO Change to async connect for better timeout handling.
	while (iter != end) {
		ip::tcp::endpoint endpoint = *iter++;
		LOG_DEBUG(logger, "Connecting to " << endpoint );
		sockt->connect(endpoint, ec);
		if (!ec)
			break;
	}

	// preset name, but only needed if we gonna log on these levels.
	if (logger.IsEnabled(ILogger::ERROR)
		|| logger.IsEnabled(ILogger::DEBUG))
		cfghelper.GetConfig("name", strhost);

	if (ec) {
		LOG_ERROR(logger, "Connection to " << strhost << " failed" );
		cmd->HandleCompletion((void *) -ECONNREFUSED);
		return true;
	}

	LOG_DEBUG(logger, "Connected to " << strhost );
	// Signal success.
	cmd->HandleCompletion((void *) 0);
	return true;

}

bool CConnectTCPAsio::HandleDisConnect( asyncCommand *cmd )
{
	boost::system::error_code ec;
	bool success = true;

	if (!IsConnected()) {
		cmd->HandleCompletion((void*) 0);
		return true;
	}

	// according boost:asio documentation, one should call shutdown before
	// close
	// (http://www.boost.org/doc/libs/1_36_0/doc/html/boost_asio/reference/basic_stream_socket/close/overload2.html)
	sockt->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
	if (ec)
		success = false;

	sockt->close(ec);
	if (ec)
		success = false;

	cmd->HandleCompletion((void*) success);
	return true;
}

/// Helping function for imeout and receive, will be called by boosts' asio.
/// this handler just will set the int store with the value value.
static void boosthelper_set_result( int* store, int value )
{
	if (store)
		*store = value;
}

/** Handle Receice -- asynchronous read from the asio socket with timeout.
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
 * HandleReceive expects a std::string in asyncCommand->auxData.
 */

bool CConnectTCPAsio::HandleReceive( asyncCommand *cmd )
{
	boost::system::error_code ec, handlerec;

	volatile int result_timer = 0;
	size_t bytes;
	unsigned long timeout;
	char buf[2];
	struct asyncReadHandler read_handler(&bytes, &handlerec);
	// timeout setup
	ioservice->reset();

	CConfigHelper cfghelper(ConfigurationPath);
	cfghelper.GetConfig("tcptimeout", timeout, 3000UL);
	deadline_timer timer(*(this->ioservice));
	boost::posix_time::time_duration td = boost::posix_time::millisec(
		timeout);
	timer.expires_from_now(td);
	timer.async_wait(boost::bind(&boosthelper_set_result,
		(int*) &result_timer, 1));

	// socket preparation
	//  async_read. However, boost:asio seems not to allow auto-buffers,
	// so we will just read one byte and when this is available, we'll
	// check for if there are some others left
	sockt->async_read_some(boost::asio::buffer(&buf, 1), read_handler);

	size_t num = ioservice->run_one(ec);

	// ioservice error or timeout
	if (num == 0 || result_timer) {
		timer.cancel(ec);
		sockt->cancel(ec);
		LOG_TRACE(logger,"Async read timeout");
		cmd->HandleCompletion((void *) -ETIMEDOUT);
		ioservice->poll();
		return true;
	}

	timer.cancel();
	ioservice->poll(ec);

	if (*read_handler.ec) {
		LOG_DEBUG(logger,"Async read failed with ec=" << *read_handler.ec);
		cmd->HandleCompletion((void*) -EIO);
		return true;
	}

	if (1 != *read_handler.bytes) {
		LOG_DEBUG(logger,"Received "
			<< *read_handler.bytes << " but expected only 1 byte");
		cmd->HandleCompletion((void*) -EIO);
		return true;

	}

	size_t avail = sockt->available();
	size_t tmp;
	size_t numrecvd = 1;
	LOG_TRACE(logger, "There are " << avail << " bytes ready to read");
	char recved[avail + 2];
	recved[0] = buf[0];
	while (avail > 0) {
		tmp = sockt->read_some(asio::buffer(&recved[numrecvd], avail),
			ec);
		LOG_TRACE(logger, "Read " << tmp << " of these");
		avail -= tmp;
		numrecvd += tmp;
		// check if error occured.
		if (ec) {
			void *v = cmd->GetAuxData();
			// give everything we got before the error.
			((std::string *) v)->assign(recved, numrecvd);
			cmd->HandleCompletion((void*) -EIO);
			return true;
		}
	}

	void *v = cmd->GetAuxData();
	// give everything we got before the error.
	((std::string *) v)->assign(recved, numrecvd);

	cmd->HandleCompletion((void*) 0);

	return true;
}
