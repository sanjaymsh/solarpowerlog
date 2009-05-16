/*
 * InverterBase.h
 *
 * Base-Class for all Inverters to be supported.
 *
 *  Created on: May 9, 2009
 *      Author: Tobias Frost
 *
 *
 *  By the way, if you are wondering if I've got Pattern fever:
 *  I'm currently reading on design patterns and, of course,
 *  need to try them out.
 */


#ifndef INVERTERBASE_H_
#define INVERTERBASE_H_

#include <string>
#include <map>
#include "IConnect.h"
#include "interfaces/CCapability.h"

using namespace std;



/** Inverter Interface .... */
// TODO: This class renamed, as it also fits for the "Filters" (Data source, data computing/enhancing, ...)
// Inverters will be only a special interface, derived from this base class
class IInverterBase {
public:

	IInverterBase( std::string name = "unnamed" );
	virtual ~IInverterBase();

	// Class for handling the connectivity.
	// Is a strategy design pattern interface. So different ways of connection can be handled by the same interface
	// (In other words: Our inverter does not want to know, if it is RS485 or TCP/IP, or even, both.)
	// NOTE: The Connection is to be made by a factory!
	// NOTE2: Beware: Connections can die any time! Make sure to handle this.
	IConnect *connection;

	// ############### GETTERS AND SETTERS ##################
	const std::string& GetName(void) const;


	// ############### COMMANDEE ###########################
	void execute(void);

	// ######### CAPABILITIES HANDLING ##########
	/** returns a iterator of the Capabilties. The iterator is inizialized at the begin of the map.*/
	virtual map<string, CCapability*>::iterator GetCapabilityIterator(void)
	{
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

	/** check for a specific capability and return the pointer to it
	 * returns NULL if it is not registered. */
	virtual CCapability *GetConcreteCapability(const string &identifier) {
			map<string, CCapability*>::iterator it = CapabilityMap.find(identifier);
			if(it == CapabilityMap.end() ) return 0;
			return it->second;
	}


private:
	/** Inverter Name -- as in config */
	std::string name;

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
