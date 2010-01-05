/*
 * CFormaterWebRootStrip.cpp
 *
 *  Created on: Jan 4, 2010
 *      Author: tobi
 */

#include "DataFilters/HTMLWriter/formatter/IFormater.h"
#include "DataFilters/HTMLWriter/formatter/CFormaterWebRootStrip.h"
#include "configuration/CConfigHelper.h"

CFormaterWebRootStrip::CFormaterWebRootStrip()
{
}

CFormaterWebRootStrip::~CFormaterWebRootStrip()
{
	// TODO Auto-generated destructor stub
}

bool CFormaterWebRootStrip::Format(const std::string &what, std::string &store,
		const std::vector<std::string> &parameters)
{
	std::string strip;
	size_t pos;

	if (parameters.size() < 3) {
		strip = "/var/www";
	} else {
		strip = parameters[2];
	}

	// basic check: what must be bigger than strip.
	if (strip.length() > what.length())
		return false;

	// make sure that the string is there
	pos = what.find(strip);

	if (pos != std::string::npos) {
		store = what.substr(strip.length(), std::string::npos);
		return true;
	}

	return false;
}

