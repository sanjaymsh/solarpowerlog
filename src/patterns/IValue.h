/** \file IValue.h
 *
 *  Created on: May 13, 2009
 *      Author: tobi
 */

#ifndef ICAPABILITY_H_
#define ICAPABILITY_H_

#include <string>
#include "IObserverSubject.h"



/** A Value is like aconcrete measurements, states, etc.
 *
 * What it makes tricky is. that they might need different data types for storage.
 *
 * TODO DOCUMENT ME!
 */
class IValue {
public:

	enum factory_types
	{
		int_type,
		float_type,
		string_type
	};

	static IValue* Factory(const factory_types typedescriptor);

	virtual factory_types GetType(void) const;

protected:
	IValue();
public:
	virtual ~IValue();

protected:
	factory_types type;

};





#endif /* ICAPABILITY_H_ */
