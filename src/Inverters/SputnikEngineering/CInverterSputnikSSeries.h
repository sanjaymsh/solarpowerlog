/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CInverterSputnikSSeries.h), the license terms are:

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

/** \file CInverterSputnikSSeries.h
 *
 *  Created on: May 21, 2009
 *      Author: tobi
 */

#ifndef CINVERTERSPUTNIKSSERIES_H_
#define CINVERTERSPUTNIKSSERIES_H_

/** \fixme COMMENT ME
 *
 *
 * TODO DOCUMENT ME!
 */
#include "InverterBase.h"

class CInverterSputnikSSeries: public IInverterBase {
public:
	CInverterSputnikSSeries(const string & name, const string & configurationpath);
	virtual ~CInverterSputnikSSeries();

	virtual bool CheckConfig();

private:
	/* calculate the checksum for the telegramm stored in str.
	 * note: */
	unsigned int CalcChecksum(const char* str, int len);

};

#endif /* CINVERTERSPUTNIKSSERIES_H_ */
