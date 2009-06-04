/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (IDataFilter.h), the license terms are:

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

/** \file IDataFilter.h
 *
 *  Created on: Jun 1, 2009
 *      Author: tobi
 *
 *   Interface for all filters, data enhancers and finally all data-sinks
 *   (which do not implement the ObserverSubject pattern)
 *
 *   How it works:
 *   - A filter, when instanciatesd, gets its configuration its own name
 *   - Using that configuration, it connects to the data source (type IInverterBase)
 *     and subscribes at least to the CAPA Notifications of the data source
 *   - the filter is aware, that most likely the capabilites will change over
 *     runtime. It will only serve the
 *   - If a cdownsteram filter queries for something which is not
 *     modified or created, the request is simply forwarded upstream
 */

#ifndef IDATAFILTER_H_
#define IDATAFILTER_H_

/** \fixme COMMENT ME
 *
 *
 * TODO DOCUMENT ME!
 */
#include "IObserverObserver.h"
#include "IObserverSubject.h"
#include "Inverters/interfaces/InverterBase.h"
#include "patterns/ICommand.h"

class IDataFilter:
	public IObserverObserver,
	public IInverterBase // The inverter, though the capabilites, also provides the
	// interface for being a subject (observer pattern)
{
public:
	IDataFilter(const string &name, const string & configurationpath)
		: IInverterBase(name, configurationpath)
	{ };
	virtual ~IDataFilter();

	// Filter needs not to use the CommandQueue facility, so defaulting to
	// doing nothing
	virtual void ExecuteCommand( const ICommand *cmd) {};

	// Just a reminder: You need to override that.
	virtual bool CheckConfig() = 0;

	// Just another reminder. Update is called by the subject,
	// when new data arrives. So you defintly want to override this.
	virtual void Update(const IObserverSubject *subject) = 0;

protected:
	/// Inverter to connect to. Can also be a another DataFilter
	/// (as data are exchanged over the IInverterBase Interface,
	/// this does not matter.
	/// Note: The child has to set this to the proper value.
	/// If the child wants to reveive data from multiple / different sources,
	/// it also might use its own implementation
	IInverterBase *base;

};




#endif /* IDATAFILTER_H_ */
