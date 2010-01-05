/*
 * IFormater.cpp
 *
 *  Created on: Jan 4, 2010
 *      Author: tobi
 */

#include "DataFilters/HTMLWriter/formatter/IFormater.h"
#include "DataFilters/HTMLWriter/formatter/CFormaterWebRootStrip.h"
#include "DataFilters/HTMLWriter/formatter/CFormatterSearchCSVEntry.h"

IFormater *IFormater::Factory(const std::string &spec)
{
	if (spec == "stripwebroot") {
		return new CFormaterWebRootStrip();
	}

	if (spec == "searchcvsentry") {
		return new CFormatterSearchCSVEntry();
	}
	return NULL;
}
