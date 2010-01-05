/*
 * CFormaterWebRootStrip.h
 *
 *  Created on: Jan 4, 2010
 *      Author: tobi
 */

#ifndef CFORMATERWEBROOTSTRIP_H_
#define CFORMATERWEBROOTSTRIP_H_

#include "DataFilters/HTMLWriter/formatter/IFormater.h"

/// Formater for: Strip a webroot-path from the front of a capability
class CFormaterWebRootStrip: public IFormater
{

public:
	virtual ~CFormaterWebRootStrip();

	virtual bool Format(const std::string &what, std::string &store,
			const std::vector<std::string> &parameters);

protected:
	// make the factory class my friend to instanciate the object.
	friend class IFormater;
	CFormaterWebRootStrip();

};

#endif /* CFORMATERWEBROOTSTRIP_H_ */
