/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (IValue.h), the license terms are:

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

/** \file InverterBase.h
 *
 *  \date   May 9, 2009
 *  \author Tobias Frost
 *
 * This is the interface for inverters.
 *
 * \page IInverterBase IInverterBase: Interface for Inverters
 *
 * IIinverterBase is the interface which should be used for all inverters.
 * It is also the base class for all DataFilters, (IDataFilter), as they
 * also need most of the infracstructure for their duties.
 *
 * It is ought to be derived, and the concrete implemenation implements the
 * data aquisition.
 *
 * To help avoiding doublicated code, the base class have contains already some
 * code for the following funcions:
 *	- <b>\ref IIBConnection "Comms"</b>
 *		- Generate the concrete connection object (IConnect
 *		  derived) The type of connection is extracted automatically
 *		  from the configuration. On Datafilters, a default "Dummy"
 *		  Connection will be created, if the specific configuratoin
 *		  entries are missing.
 *	- <b>\ref IIBCapabilites "Capabilites"</b>
 *		- Generating the required CCapabilites (see
 *		  #Inverters/Capabilites.h)
 *		- Infrastructure around Capabilites: Adding, Iterator, Finding
 *	          an concrete Capability...
 *	 - <b> \ref IIBWorkScheduler "Target for a WorkScheduler" </b>
 *	 	- Inverters should use solarpowerlog's Work Scheduler for
 *	 	  execution planning. To be able to receive ICommands they are
 *	 	  derived from ICommandTarget.
 *	        - By using the default Scheduler (see #Registry), concurrency
 *	          is avoided, so there is no need for locking methods for common
 *	          accessed datas.
 *
 * \section IIBConnection Connection: Connecting to the word.
 *
 * As stated before, the base class creates the communication object.
 * To determine which connectiion type is to be used, it will use the factory
 * IConnectionFactory. This factory looks in the configuration-path -- supplied
 * by the bootstrap code -- for the entry "commtype" and create the associated
 * comms object.
 *
 * BTW: The connection classes will also retrieve their configuration from the
 * config file automatically.
 *
 * The comms object will <b>not</b> automatically connect. This should be done
 * later while running the initializtion commmand (see workqueue below)
 *
 * \image html inverter_comm.png "Connection Services"
 *
 * The image above shows the services currently exists for the comm classes.
 *
 * Currently, the connection classed are mixed synchron and asynchronous. But
 * this will change, the connection class should use asynchronous
 * methods to avoid blocking the main task while waiting for completion.
 *
 * This also implies, that the inverters have to implement some kind of time-out
 * handling by itself. (Well its on the TODO list: \todo implement some kind of
 * generic timeout handling, like schedule timeout, cancel timeout.)
 *
 * \sa #IInverterBase::connection
 *
 * \section IIBCapabilites Capabilites
 *
 * As described \ref CapaConcept "here", Capabilies store abstraced measurement
 * values as well as some meta-datas controling the Capabilites itself.
 *
 * Capabilites are always created at the data-source. The Datasource links them
 * into its Capability-map. By this, it offers the data to the data-sinks, the
 * DataFilters or Loggers. The Loggers which are connected to the Inverter, can
 * Subscribe to the information and then will get informed when the data has
 * been updated.
 *
 * Datafilters can also be chained. With this chaning, abitrary data
 * manipulations can be achieved: The datafilter can calculate additional data
 * sets (example: Efficiency out of Pin and Pout), or do some other arithmetics
 * on them (mean value...)
 *
 * Datafilters can also be parallel to e.g enable parallel serving of data to
 * different logging systems: One would log to disk, the other servin a web
 * page.
 *
 * ( \todo More details are planned to appear on the IDataFilter description.)
 *
 * The image below shows all the classes involved in the dataflow from the
 * Inverter to the DataFilter:
 *
 * \image html dataflow_classes.png "Classes needed to let the data flow"
 *
 * This image shows how a typical update of a Capbility, and how the Data-Filter
 * receives the information.
 *
 * \image html dataflow_sequence.png "Dataflow Sequence Diagram"
 *
 * \subsection IIBCSnip Code-Snippets around Capabilites.
 *
 * <b> Unconditionally Register a new Capability </b>(e.g in the constructor)
 * \code
 * 	c = new CCapability(CAPA_CAPAS_UPDATED, CAPA_CAPAS_UPDATED_TYPE, this);
 *	AddCapability(CAPA_CAPAS_UPDATED, c);
 * * \endcode
 *
 * <b> Update a Capability's Value, create if not existance. Check for type </b>
   \code
 	CCapability *cap =
 		GetConcreteCapability(CAPA_INVERTER_TEMPERATURE_NAME);

	if (!cap) {
		string s;
		IValue *v;
		CCapability *c;
		s = CAPA_INVERTER_TEMPERATURE_NAME;
		v = IValue::Factory(CAPA_INVERTER_TEMPERATURE_TYPE);
		((CValue<float>*) v)->Set(f);
		c = new CCapability(s, v, this);
		AddCapability(s, c);

		// TODO: Check if we schould derefer (using a scheduled work) this.
		cap = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		cap->Notify();
	}
	// Capa already in the list. Check if we need to update it.
	else if (cap->getValue()->GetType() == CAPA_INVERTER_TEMPERATURE_TYPE) {
		CValue<float> *val = (CValue<float>*) cap->getValue();

		if (val -> Get() != f) {
			val->Set(f);
			cap->Notify();
		}
	} else {
		cerr << "BUG: " << CAPA_INVERTER_TEMPERATURE_NAME
			<< " not a float ";
	}
 \endcode
 *
 *
 * \section IIBWorkScheduler "Work Scheduler"
 *
 * IInverterbase is derived from ICOmmandTarget. This class can reiceive
 * commands from CWorkSchedule objects. One CWorkSchedule object can be received
 * by the #Registry, which is open for all objects to use. (This Scheduler is
 * operated by solarpowerlogs internals, so no extra code is required to have
 * an operating schedule.)
 *
 * For Details, for example, how to use a CWorkSchedule and how to schedule
 * some Work in a specific amount of time, please see the \ref PageCWorkSchedule
 * "WorkScheduler overview" Page.
 *
 * \todo which is not yet written,
 *
 *
 *
 */

#ifndef INVERTERBASE_H_
#define INVERTERBASE_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <map>

#include "interfaces/IConnect.h"
#include "interfaces/CCapability.h"
#include "patterns/ICommandTarget.h"

class ICapaIterator;

using namespace std;

/** Inverter Interface .... */
// TODO: This class renamed, as it also fits for the "Filters" (Data source, data computing/enhancing, ...)
// Inverters will be only a special interface, derived from this base class
class IInverterBase : public ICommandTarget
{

public:
	friend class ICapaIterator;

	IInverterBase( const string &name, const string & configurationpath );
	virtual ~IInverterBase();

	virtual const std::string& GetName( void ) const;

	/** check for a specific capability and return the pointer to it
	 * returns NULL if it is not registered. */
	// TODO Move to c++ file
	virtual CCapability *GetConcreteCapability( const string &identifier );

	virtual ICapaIterator* GetCapaNewIterator();

	virtual bool CheckConfig() = 0;

protected:

	virtual void AddCapability( const string &id, CCapability* capa );

	/** returns a iterator of the Capabilties. The iterator is inizialized at the begin of the map.*/
	virtual map<string, CCapability*>::iterator
		GetCapabilityIterator( void );

	/** return a iterator of the Capabilites. The iterator is placed at the end of the map
	 * This allows a end-of-list check */
	virtual map<string, CCapability*>::iterator GetCapabilityLastIterator(
		void );

	/** Configuration path as determined on start -- for easier fetching the config.*/
	std::string configurationpath;
	/** Inverter Name -- as in config */
	std::string name;

	// Class for handling the connectivity.
	// Is a strategy design pattern interface. So different ways of connection can be handled by the same interface
	// (In other words: Our inverter does need nor want to know, if it is RS485 or TCP/IP, or even, both.)
	// NOTE: The Connection is to be made by a factory!
	// NOTE2: Beware: Connections can die any time! Make sure to handle this.
	IConnect *connection;

private:

	// This maps contains all the Caps by the Inverter.
	// Caps implements the Subject in the Observer-Pattern.
	// This keeps all informed!
	// Class for handling capabilities.
	// (Inverters can add capabilities at runtime, also can enhancement filters.
	// (A Capabilites bundles on discret feature, reading of a inverter,
	// like also current readings and so on...

	map<string, CCapability*> CapabilityMap;
};

#endif /* INVERTERBASE_H_ */

// debug code to keep...

// Dumping the map.
//		map<string, CCapability*>::iterator it1 = CapabilityMap.begin();
//		cerr << "DUMP Capabilites: " << this->GetName() <<  endl;
//		for (int i=0; it1 != CapabilityMap.end(); it1++,i++)
//		{
//			cerr << i << " " << (*it1).first << endl;
//		}
