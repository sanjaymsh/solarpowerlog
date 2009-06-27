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
 * \page IInverterBase IInverter: Data-Flow
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
class IInverterBase : public ICommandTarget {

public:
	friend class ICapaIterator;

	IInverterBase( const string &name, const string & configurationpath );
	virtual ~IInverterBase();

	virtual const std::string& GetName(void) const;


protected:
	/** returns a iterator of the Capabilties. The iterator is inizialized at the begin of the map.*/
	virtual map<string, CCapability*>::iterator GetCapabilityIterator(void)
	{
		// Dumping the map.
//		map<string, CCapability*>::iterator it1 = CapabilityMap.begin();
// TODO cleanup code (remove debug code)
//		cerr << "DUMP Capabilites: " << this->GetName() <<  endl;
//		for (int i=0; it1 != CapabilityMap.end(); it1++,i++)
//		{
//			cerr << i << " " << (*it1).first << endl;
//		}

		map<string, CCapability*>::iterator it = CapabilityMap.begin();
		return it;
	}

	/** return a iterator of the Capabilites. The iterator is placed at the end of the map
	 * This allows a end-of-list check */
	virtual map<string, CCapability*>::iterator GetCapabilityLastIterator(void)
	{
		map<string, CCapability*>::iterator it = CapabilityMap.end();
		return it;
	}

public:
	/** check for a specific capability and return the pointer to it
	 * returns NULL if it is not registered. */
	// TODO Move to c++ file
	virtual CCapability *GetConcreteCapability(const string &identifier) {
			map<string, CCapability*>::iterator it = CapabilityMap.find(identifier);
			if(it == CapabilityMap.end() ) return 0;
			return it->second;
	}

	virtual ICapaIterator* GetCapaNewIterator();

	virtual bool CheckConfig() = 0;

protected:

	virtual void AddCapability(const string &id, CCapability* capa)	{
		CapabilityMap.insert( pair<string,CCapability*>(id,capa));
		cerr << "Added new Capability to " << name << ": " << id << endl;
	}

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

	/ This maps contains all the Caps by the Inverter.
	// Caps implements the Subject in the Observer-Pattern.
	// This keeps all informed!
	// Class for handling capabilities.
	// (Inverters can add capabilities at runtime, also can enhancement filters.
	// (A Capabilites bundles on discret feature, reading of a inverter,
	// like also current readings and so on...

	map<string, CCapability*> CapabilityMap;
};

#endif /* INVERTERBASE_H_ */
