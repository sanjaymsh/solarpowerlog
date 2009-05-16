/** \file CValue.h
 *
 *  Created on: May 14, 2009
 *      Author: tobi
 *
 * Template-Class for the concrete Values.
 *
 * Note: The factory has to set IValue::type, as I don't know how to...
 */


#ifndef CVALUEX_H_
#define CVALUEX_H_


#include "IValue.h"

template <class T>
class CValue : public IValue {

public:
	CValue () { }

	void Set( T value) {this->value = value;}
	T Get (void) {return value;}

private:
	T value;

};

#endif /* CVALUEX_H_ */
