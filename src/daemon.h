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

/** \file signalhandler.h
 *
 *  Created on: 01.10.2012
 *      Author: tobi
 *
 * Bundles the Daemon and Signalhandler functionalities.
 * Stores also the relevant data variables.
 */

#ifndef SIGNALHANDLER_H_
#define SIGNALHANDLER_H_

#include <string>
#include <signal.h>
#include <boost/thread.hpp>

/// Filename for the pidfile.
extern std::string pidfile;

/// Directory where to chdir to.
extern std::string rundir;
/// File where to redirect stdout
extern std::string daemon_stdout;
/// File where to redirect stderr
extern std::string daemon_stderr;

/// Termination request received.
extern volatile sig_atomic_t killsignal;
/// SIGUSR1 has been received.
extern volatile sig_atomic_t sigusr1;

/// stores the programmame (argv[0])
extern char *progname;

/// Filedescriptor for pid-file
/// Note: pid file will be closed right after generation, then this will be kept
/// non-zero to indicate that we have a pid file and need to unlink it on exit.
extern int pidfile_fd;

/// Should solarpowerlog run in the background?
extern bool background;

/// Perform some cleanups at exit
extern void cleanup(void);

/// rotate logfiles (reopening)
void logreopen(bool rotate);

/// Daemonizes solarpowerlog
extern void daemonize(void);

/// The signal handler
extern void SignalHandler(int signal);

/// Setup the signal handler
/// must be called from main
extern void SetupSignalHandler(void);

/// This thread is spawn when receiving SIGTERM and the mutex in the CWorkScheduler
/// was locked at that time
extern boost::thread terminator_thread;

#endif /* SIGNALHANDLER_H_ */
