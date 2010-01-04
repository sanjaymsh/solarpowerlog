/*
 * IFormater.cpp
 *
 *  Created on: Jan 4, 2010
 *      Author: tobi
 */

#include "DataFilters/HTMLWriter/formatter/IFormater.h"
#include "DataFilters/HTMLWriter/formatter/CFormaterWebRootStrip.h"

IFormater *IFormater::Factory(const std::string &spec,
		const std::string &configpath)
{
	if (spec == "stripwebroot") {
		return new CFormaterWebRootStrip(configpath);
	}

	return NULL;
}
