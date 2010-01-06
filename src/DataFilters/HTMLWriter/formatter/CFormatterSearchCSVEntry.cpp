/*
 * CFormatterSearchCSVEntry.cpp
 *
 *  Created on: Jan 5, 2010
 *      Author: tobi
 */

#include "DataFilters/HTMLWriter/formatter/CFormatterSearchCSVEntry.h"

#include <sstream>

using namespace std;

bool CFormatterSearchCSVEntry::Format(const std::string & what,
		std::string & store, const std::vector<std::string> & parameters)
{
	// the task is to get a comma-seperated value by what
	// and extract the position for the element given by the parameter
	// [2] and store the number in store.
	std::string s;

	try {
		s = parameters[2];
	} catch (...) {
		return false;
	}

	// What we got:
	// value, value2, value3
	// We count the seperators, and we are set.

	size_t pos = what.find(s);

	if (pos == std::string::npos)
		return false;

	int count = 0;
	for (size_t i = 0; i < pos; i++) {
		if (what[i] == ',' && (i == 0 || what[i - 1] != '\\')) {
			count++;
		}
	}

	std::stringstream ss;
	ss << count;
	ss >> store;

	return true;
}

CFormatterSearchCSVEntry::CFormatterSearchCSVEntry()
{
	// TODO Auto-generated constructor stub

}

CFormatterSearchCSVEntry::~CFormatterSearchCSVEntry()
{
	// TODO Auto-generated destructor stub
}
