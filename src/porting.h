/* ----------------------------------------------------------------------------
   solarpowerlog
   Copyright (C) 2009  Tobias Frost

   This file is part of solarpowerlog.

   Solarpowerlog is free software; However, it is dual-licenced
   as described in the file "COPYING".

   For this file (porting.h), the license terms are:

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

/** \file porting.h
 *
 * This file defines anything that is needed to get solarpowerlog ported other operating systems.
 *
 * Even if the componenets of solarpowerlog are carefully selected to ensure portability, some "changes" are
 * needed for some systems.
 *
 * Currently, this file is needed to enable a build under cygwin/win32.
 *
 *  Created on: Jun 24, 2009
 *      Author: tobi
 */

#ifndef PORTING_H_
#define PORTING_H_

#ifndef VERSION
#error include config.h prior this header!
#endif


/* Windows stuff : For building under cygwin. */
#ifdef HAVE_WIN32_API
#define _POSIX_SOURCE
#define _WIN32_WINNT 0x0501
#define __USE_W32_SOCKETS 1
#endif


#endif /* PORTING_H_ */
