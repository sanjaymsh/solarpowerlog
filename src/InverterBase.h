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

using namespace std;

class IConnectivity;

class ICapabilityHandler;

/** Inverter Interface .... */
// TODO: This class renamed, as it also fits for the "Filters" (Data source, data computing/enhancing, ...)
// Inverters will be only a special interface, derived from this base class
class IInverterBase, public IObserverObserver {
public:

	IInverterBase( std::string name = "unnamed" );
	virtual ~IInverterBase();

	// Class for handling the connectivity.
	// (Is a strategy design pattern interface)
	IConnectivity *connect;

	// Class for handling capabilities.
	// (Inverters can add capabilities at runtime, also can enhancement filters.
	// (A Capabilites bundles on discret feature, reading of a inverter,
	// like also current readings and so on...
	// TODO new name for class. That one does not fit, as it also should handle datas.
	ICapabilityHandler *capabilties;

	// Note: The caps are also the place where to observer pattern should reside.
	// --> This way, on a update, all can be informed.

	// Observer Pattern -- Objects can subscribe to the infomration
	// The subscription is capability-based -- if a capabilty is not availaible, return NULL
	// (note: data processors can also enhance capabilities by calculating missing values, like P = U*I..)
	//virtual bool Subscribe( class SubsciberBaseClass, class Capability_Interested_In );

	// Observer Pattern -- Objects can receive notifications.
	 //virtual bool Notify( class Capability_Subscribed);

	std::string GetName();


private:
	/** Inverter Name -- as in config */
	std::string name;




};

#endif /* INVERTERBASE_H_ */
