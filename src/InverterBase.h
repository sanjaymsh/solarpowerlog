/*
 * InverterBase.h
 *
 * Base-Class for all Inverters to be supported.
 *
 *  Created on: May 9, 2009
 *      Author: Tobias Frost
 */


#ifndef INVERTERBASE_H_
#define INVERTERBASE_H_

#include <string>


class InverterBase {
public:

	InverterBase( std::string name = "unnamed" );
	virtual ~InverterBase();

	virtual bool Connect() = 0;
	virtual bool Disconnect() = 0;

	std::string GetName();


private:
	/// Gives the cinverter a name:
	std::string name;




};

#endif /* INVERTERBASE_H_ */
