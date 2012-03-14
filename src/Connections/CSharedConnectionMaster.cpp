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


/*
 * CSharedConnectionMaster.cpp
 *
 *  Created on: Sep 13, 2010
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_COMMS_SHAREDCONNECTION

#include "configuration/Registry.h"
#include "CSharedConnectionMaster.h"
#include "Connections/factories/IConnectFactory.h"
#include "configuration/Registry.h"

#include <boost/date_time.hpp>
#include "configuration/CConfigHelper.h"

enum
{
	CMD_READ, CMD_CHECKTIMEOUTS,
};

// THOUGHTS:
/*
 * Problems to solve:
 * TIMEOUTs
 *    * slaves has to keep track of its timeouts, as the dispatched call to
 *      the master can timeout before/afterwards
 *
 *          - master keeps track of all timeouts,
 *          - the slaves add the information required
 *            for the timeouts to the command, if they are not present.
 *          - master tracks the timeout horizon for all icommands, and if timeout
 *            is met before a receive is completed, the receive fails.
 *          - if the "real connection object" receive times out, it is checked
 *            if others are not yet timed out, therefore requesting another receive
 *            with the remaining timeout time.
 *
 *            PROBLEM: Intercommand races. As FIFO for comm is used, it could be that
 *            a receive command and its resume could be not adjacent in the queue,
 *            but interrupted with a "send", for example.
 *
 *            SOLUTION: a) Needing a command queue in the master which will timestamp
 *            the commands received and makes sure that the next commmand is only
 *            execeuted when all "receives" are indeed completed
 *            b) as a, but only for receive handling completion... If receiving, all other
 *            commands are delayed until timeouts occures.
 *            b) ignore for now...
 *
 *
 *
 * CONNECT/DISCONNECT
 *    * Masters/Slaves might want to disconnect/connect according to their logic,
 *      but constant disconnection/connection might jeopadize everything.
 *
 *
 *
 *
 * old:
 * - the master talks to the real connection class
 * - (the first version) will just do a FIFO in comms, so getting the
 *   requests from the inverter, adding them to an internal list and
 *   then propagate them to the comms object, acting as a negotiater.
 * - it cannot just add them to the targets comms queue, as receive events
 *   might impose race conditions so that a answer would be routed to a wrong
 *   destiation or a message would be concentreated so that a inverter gets its
 *   own and a foreign one (or in the other sequence)
 *   Another issue is that there might be inverters that do not need an question
 *   to answer, but keep talking without request. The Connection class could and
 *   should not have logic to sort them apart.
 *   This is solved that all inverters currently waiting for receiption will
 *   get the answer and as they have the knowledge about the telegramms, they
 *   easily can sort out the other telegrams.
 *
 * */

CSharedConnectionMaster::CSharedConnectionMaster(
		const string & configurationname) :
	IConnect(configurationname)
{
	connection = NULL;
}

CSharedConnectionMaster::~CSharedConnectionMaster()
{
	if (connection)
		delete connection;
}

void CSharedConnectionMaster::ExecuteCommand(const ICommand *Command)
{

//	Command->DumpData(logger);


	switch (Command->getCmd())
	{

	// A receive command has been issued with a shorter timeout than
	// the current read command.
	// So it could be that we need to time-out something....
	case CMD_CHECKTIMEOUTS:
	{
		boost::posix_time::ptime now(
				boost::posix_time::microsec_clock::local_time());

		boost::posix_time::ptime timeout;

		// check all pending reads for timeout.
		list<ICommand*>::iterator it = readcommands.begin();
		while (it != readcommands.end()) {
			ICommand *trgt = *it;
			it++; // increment to be sure that we can remove the item after evaluation.

			try {
				timeout = boost::any_cast<boost::posix_time::ptime>(
						trgt->findData(ICONNECT_TOKEN_TIMEOUTTIMESTAMP));
			} catch (std::invalid_argument &e) {
				LOGDEBUG(logger,"BUG: Required timeout-key not found. " <<
						__PRETTY_FUNCTION__ << " at " << __LINE__ << " what:" << e.what());
				timeout = now;
			} catch (boost::bad_any_cast &e) {
				LOGDEBUG(logger,"BUG: Bad boost::any-cast at " <<
						__PRETTY_FUNCTION__ << ", " << __LINE__ << " what:"
						<< e.what());
				timeout = now;
			}

			if (timeout <= now) {
				trgt->addData(ICMD_ERRNO, -ETIMEDOUT);
				readcommands.remove(trgt);
				Registry::GetMainScheduler()->ScheduleWork(trgt);
			}
		}

		// Saftey: Check is list is empty... (Should not happen here anyway)
		if (readcommands.empty())
			readtimeout = boost::posix_time::not_a_date_time;

		break;
	}

		// A read command issued to the real connection object has returned.
		// Evaluate its status and if successful return the retrieved object
		// to all listeners.
		// On timeouts check if others are waiting for a longer time and re-issue
		// the receive in this case.
		// On other errors, inform the callers for error handling.
	case CMD_READ:
	{
		long errorcode;

		try {
			errorcode = boost::any_cast<int>(Command->findData(ICMD_ERRNO));
		} catch (std::invalid_argument &e) {
			LOGDEBUG(logger,"BUG: Required parameter not found." <<
					__PRETTY_FUNCTION__ << " at " << __LINE__ << " what:" << e.what());
			errorcode = -EIO;

		} catch (boost::bad_any_cast &e) {
			LOGDEBUG(logger,"BUG: Required parameter cast failed " <<
					__PRETTY_FUNCTION__ << " at " << __LINE__ << " what:" << e.what());
			errorcode = -EIO;
		}

		if (errorcode != ETIMEDOUT) {
			// Successful read. Distribute received data to all listeners.
			list<ICommand*>::iterator it;
			for (it = readcommands.begin(); it != readcommands.end(); it++) {
//				LOGDEBUG(logger, "Premerge:");
//				(*it)->DumpData(logger);
//				LOGDEBUG(logger, "Merge with:");
//				Command->DumpData(logger);
				(*it)->mergeData(*Command);
//				LOGDEBUG(logger, "Merged");
//				(*it)->DumpData(logger);
				Registry::GetMainScheduler()->ScheduleWork(*it);
			}
			readcommands.clear();
			readtimeout = boost::posix_time::not_a_date_time;
			break;

		} else /* if (errorcode == ETIMEDOUT) */ {
			// timeout.
			// notifiy and calculate the maximum waiting time for the next
			// receive command.

			boost::posix_time::ptime now(
					boost::posix_time::microsec_clock::local_time());

			boost::posix_time::ptime timeout;

			boost::posix_time::ptime largest_timeout = now;

			list<ICommand*>::iterator it = readcommands.begin();
			while (it != readcommands.end()) {
				ICommand *trgt = *it;
				it++; // increment to be sure that we can remove the item after evaluation.

				try {
					timeout = boost::any_cast<boost::posix_time::ptime>(
							trgt->findData(ICONNECT_TOKEN_TIMEOUTTIMESTAMP));
				} catch (std::invalid_argument &e) {
					LOGDEBUG(logger,"BUG: Required timeout-key not found. " <<
							__PRETTY_FUNCTION__ << " at " << __LINE__ << " what:" << e.what());
					timeout = now;
				} catch (boost::bad_any_cast &e) {
					LOGDEBUG(logger,"BUG: Bad boost::any-cast at " <<
							__PRETTY_FUNCTION__ << ", " << __LINE__ << " what:"
							<< e.what());
					timeout = now;
				}

				if (largest_timeout < timeout) {
					largest_timeout = timeout;
				}

				if (timeout <= now) {
					trgt->addData(ICMD_ERRNO, -ETIMEDOUT);
					readcommands.remove(trgt);
					Registry::GetMainScheduler()->ScheduleWork(trgt);
				}
			}

			if (readcommands.empty()) {
				// If all reads are gone, reset readtimeout.
				readtimeout = boost::posix_time::not_a_date_time;
			} else {
				// Make a copy of the current command to reuse for the next
				// receive (the lifetime of the current one ends after this
				// function returns.)
				// Schedule the largest known timeout, the shorter ones are
				// handled in the CHECKTIMEOUTs state.
				ICommand *cmd = new ICommand(*Command);
				boost::posix_time::time_duration dur = largest_timeout - now;
				long remainingtimeout = dur.total_milliseconds();
				cmd->addData(ICONN_TOKEN_TIMEOUT, remainingtimeout);
				connection->Receive(cmd);
				readtimeout = largest_timeout;
			}

		}

		break;
	}
	}

}

void CSharedConnectionMaster::Connect(ICommand *callback)
{
	assert(connection);
	assert(callback);
	connection->Connect(callback);
}

void CSharedConnectionMaster::Disconnect(ICommand *callback)
{
	assert(connection);
	assert(callback);
	connection->Disconnect(callback);
}

void CSharedConnectionMaster::SetupLogger(const string& parentlogger,
		const string &)
{
	IConnect::SetupLogger(parentlogger, "");

	if (connection)
		connection->SetupLogger(parentlogger, "");
}

void CSharedConnectionMaster::Send(ICommand *callback) {
	assert(callback);
	assert(connection);
	connection->Send(callback);
}

void CSharedConnectionMaster::Receive(ICommand *callback)
{
	assert(callback);

#warning need we to hold a mutex here?

	// Save command for later interpretation.
	readcommands.push_back(callback);

	boost::posix_time::ptime timestamp(
			boost::posix_time::microsec_clock::local_time());

	// Add a timestamp for the timeout handling, if it is not already there
	try {
		// just check if it is there.
		timestamp = boost::any_cast<boost::posix_time::ptime>(
				callback->findData(ICONNECT_TOKEN_TIMEOUTTIMESTAMP));
	} catch (...) {
		// Apparently not, or the caller did not add an ptime....
		// get timeout from config and calculate the timestamp.
		// for now, we can only use our config, as this is the best knowledge
		// we have, (otherwise we have a default configuration....)

		unsigned long timeout;

		try {
			timeout = boost::any_cast<long>(callback->findData(
					ICONN_TOKEN_TIMEOUT));
		} catch (...) {
			CConfigHelper cfg(ConfigurationPath);
			cfg.GetConfig("timeout", timeout, SHARED_CONN_MASTER_DEFAULTTIMEOUT);
		}

		timestamp += boost::posix_time::millisec(timeout);
		callback->addData(ICONNECT_TOKEN_TIMEOUTTIMESTAMP, timestamp);
	}

	// check if the new command is supposed to timeout earlier than
	// the current read request. In this case, add an additional worker
	// to get notified in time.
	if (readtimeout == boost::posix_time::not_a_date_time) {
		// not a pending read.
		// clone the command, divert its response to this class and the read
		// to the real connection class.
		// If no read is pending, directly issue a read commmand to the connection,
		// but divert result to our class for later distribution.
		// for this, we copy-construct a ICOmmand and modify it afterwards.

		this->readtimeout = timestamp;
		ICommand *cmd = new ICommand(*callback);
		callback->addData(ICONNECT_TOKEN_PRV_ORIGINALCOMMAND, callback);
		cmd->setTrgt(this);
		cmd->setCmd(CMD_READ);
		connection->Receive(cmd);
	}

	// a receive already pending. Schedule work to monitor its timeout.
	ICommand *cmd = new ICommand(CMD_CHECKTIMEOUTS, this);
	boost::posix_time::time_duration duration;
	duration = timestamp - boost::posix_time::microsec_clock::local_time();
	struct timespec ts;
	ts.tv_sec = duration.total_seconds();
	ts.tv_nsec = duration.total_nanoseconds() % 1000000000;
	Registry::GetMainScheduler()->ScheduleWork(cmd, ts);
}

bool CSharedConnectionMaster::CheckConfig(void)
{

	// Get real configuration path to extract target comms config.
	string commsconfig = this->ConfigurationPath + ".realcomms";
	string s;
	CConfigHelper h(commsconfig);

	if (! h.GetConfig("comms",s)) {
		LOGERROR(logger,"realcomms section: comms missing");
		return false;
	}

	connection = IConnectFactory::Factory(commsconfig);

	if (connection) {
		connection->SetupLogger(ConfigurationPath,"realcomms");
		return connection->CheckConfig();
	} else {
		LOGERROR(logger,"No connection object for shared master comms.");
		return false;
	}
}

bool CSharedConnectionMaster::IsConnected(void)
{
	assert(connection);
	return connection->IsConnected();
}

#endif
