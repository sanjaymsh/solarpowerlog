/*
 * CFormatterSearchCSVEntry.h
 *
 *  Created on: Jan 5, 2010
 *      Author: tobi
 */

#ifndef CFORMATTERSEARCHCSVENTRY_H_
#define CFORMATTERSEARCHCSVENTRY_H_

#include "DataFilters/HTMLWriter/formatter/IFormater.h"

class CFormatterSearchCSVEntry: public IFormater
{

public:
	virtual ~CFormatterSearchCSVEntry();

	virtual bool Format(const std::string &what, std::string &store,
			const std::vector<std::string> &parameters);

protected:
	// make the factory class my friend to instanciate the object.
	friend class IFormater;
	CFormatterSearchCSVEntry();

};

#endif /* CFORMATTERSEARCHCSVENTRY_H_ */
