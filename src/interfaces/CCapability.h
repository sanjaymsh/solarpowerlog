/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CCapability.h), the license terms are:

 You can redistribute it and/or  modify it under the terms of the GNU Lesser
 General Public License (LGPL) as published by the Free Software Foundation;
 either version 3 of the License, or (at your option) any later version.

 This progrmm is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/** \file CCapability.h
 *
 *  \date Created on: May 16, 2009
 *  \author Tobias Frost (coldtobi)
*/

/** \page CapaConcept "The Capabilites Concept"
 *
 * This gives an introduction how capabilites works and whats their purpose is.
 * For details, please also see the relevant interfaces, mostly CCapability and
 * IValue.
 *
 * Further information can also be found on the chapter over Inverters and
 * DataFilters.
 *
 * \section CapPurpose "Purpose"
 *
 * Inverters have some datas to report. These datas might be states,
 * measurements or other information. Solarpowerlog bundles these kind of
 * information into one object type, a Capability. The bundle contains as well
 * the name as also the associated value.
 *
 * The name of a capability is used to identified the kind of information
 * stored. This way most information are abstraced and brought to the same
 * common denominator:
 *
 * The inverter (data source) sets up the value and the receiver of the data can
 * query for it by the nameas know exactly how to interpret it, as this is
 * exactly defined.
 *
 * The inverter is not required to set up the complete set of information -- it
 * will just give the infos it has. On the other side, the receiver can not
 * expect that all information are always available.
 *
 * Even if this increases complexity of the receivers implementation -- it has
 * to make the features it has dependent on the information diveristy it
 * gets -- it also makes it far more flexible: The program will work on
 * inverters which delivers many information as well with inverters which will
 * deliver only a basic set.
 *
 * Also, if a inverter decides to give even more information, this data can
 * easily be added: The receiver which do not know about the infos won't care,
 * and the others can make use of it.
 *
 * Another use-case is, that data receiver can also use redudnt data sources:
 * They can reconstruct (unknown) pieces of information out of others.
 * For example, if the power in and power out is known, a filter can calculate
 * the efficeancy. If the inverter gives that value, the data receiver can use
 * that value. (actually a DataFilter is planned to make use of that...)
 *
 * \sa Please see the file Capabilities.h for some defined capabilites.
 * \sa IValue for the Value storage
 *
 * \section CapaObserverSubscriberPattern "Capability Change Notifications"
 *
 * The Capability interface is designed using the observer design pattern:
 * The Capability is the Subject and all data sources are the observers,
 * which will get notified on changes.
 *
 * Note, that this notification is not automated: The one setting the value
 * has still to call the CCapabiltiy::notify() function.
 *
 * The Subject-Pattern is implemented by inheritating from the IObserverSubject
 * interface.
 *
 * \section Cappredefined "Required Capabilites"
 *
 * In the file Capabilites.h some of the predefined Capbabilites are markes as
 * required. These means, that everyone using the interface has to implement
 * (data source) or at least is required to subscribe to this information.
 *
 * These Capabilites are helping to manage the capability system.
 *
 * \section CapaWarnings "Important Implementation Notes"
 *
 * - The class also contains a back-link to the generating inverter, using the
 *   data field source. A user of this field should always be aware, that the
 *   pointer might be NULL.
 *
 * - Even if the interface allows you, only the data-source should be change a
 *   value.
 *
 *
*/


#ifndef CCAPABILITY_H_
#define CCAPABILITY_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include "patterns/IObserverSubject.h"
#include "patterns/IValue.h"

#include <boost/utility.hpp>

class IInverterBase;
class IValue;

using namespace std;

/** Implements the CCapability Concept
 *
 * Please see the \ref CapaConcept "Capability Concept" description for details on the concepts.
 *
 * The class is noncopyable as usually a Capability is non-transferable due
 * to the assosicated Value.
 *
 * <b> OBJECTS ASSESTS (OBJECT OWNERSHIPS) </b>
 *
 * To understand object responsibility (espcially who's task it is to delete
 * that object) , and as the class contains some pointers
 * to foreign objects, it is important to define the responsbility on these
 * objects.
 *
 * 	- source <b> NOT RESPONSIBLE </b>
 * 	- value  <b> TAKES OWNERSHIP. DELETES ON DESTRUCTION. </b>
 */
class CCapability : public IObserverSubject ,
	boost::noncopyable
{
public:
	/** Construct a Capability, specify the concrete objects it should use.
	 *
	 * \param descr String used to identify the Capabilty. Should be
	 * descriptive,as it might be present to the end-user.
	 *
	 * \param val Associated Value object.
	 *
	 * \param datasrc Pointer to the generating Inverter. Can be used for
	 * datafilter to find out what the origin of the data is.
	 *
	 * */
	CCapability( const string& descr, IValue *val, IInverterBase *datasrc =
		NULL );

	/** Convenience-Constructor: Make a Capabilty and also create the value-object. */
	CCapability( const string &descr, IValue::factory_types type,
		IInverterBase *datasrc = NULL );

	/** Destructor. Deletes the value object as well. (See CCapability for details)*/
	virtual ~CCapability();

	/** Get Access to the description
	 *
	 * \returns description. */
	const string & getDescription() const
	{
		return description;
	}

	/** Get a the pointer to the one feeding this data.
	 *
	 * \returns Pointer.
	 *
	 * \warning may return NULL*/
	virtual IInverterBase *getSource() const
	{
		return source;
	}

	/** Returns the pointer t*/
	virtual IValue *getValue() const
	{
		return value;
	}

protected:
	/** storage for the description passed by the creator */
	string description;
	/** storage for the source-pointer passed by the creator */
	IInverterBase *source;
	/** storage for the value-pointer passed by the creator */
	IValue *value;
};

#endif /* CCAPABILITY_H_ */
