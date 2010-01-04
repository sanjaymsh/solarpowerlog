/*
 * CFormaterWebRootStrip.cpp
 *
 *  Created on: Jan 4, 2010
 *      Author: tobi
 */

#include "DataFilters/HTMLWriter/formatter/IFormater.h"
#include "DataFilters/HTMLWriter/formatter/CFormaterWebRootStrip.h"
#include "configuration/CConfigHelper.h"
#include <iostream>


CFormaterWebRootStrip::CFormaterWebRootStrip(const std::string & configpath) :
IFormater(configpath)
{
}

CFormaterWebRootStrip::~CFormaterWebRootStrip()
{
	// TODO Auto-generated destructor stub
}

bool CFormaterWebRootStrip::Format(const std::string & what, std::string & store)
{
	CConfigHelper ch(config);
	std::string strip;
	size_t pos;
	ch.GetConfig("webroot",strip,std::string("/var/www"));

	// basic check: what must be bigger than strip.
	if (strip.length() > what.length()) return false;

	// make sure that the string is there
	pos= what.find(strip);

	cerr << what << " ";
	cerr << strip << endl;

	if ( pos != std::string::npos )
	{
		store = what.substr(strip.length(), std::string::npos);
		return true;
	}

	return false;
}


