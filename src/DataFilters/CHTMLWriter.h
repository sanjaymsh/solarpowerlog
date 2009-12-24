/*
 * CHTMLWriter.h
 *
 *  Created on: Dec 20, 2009
 *      Author: tobi
 */

#ifndef CHTMLWRITER_H_
#define CHTMLWRITER_H_

#include "DataFilters/interfaces/IDataFilter.h"

#include "ctemplate/ctemplate.h"

/** Writes data prepared by other plugins / inverters
 * to a template, which can be HTML, for example
 *
 * This plugin does load a template, and replaces magic
 * values with the ones from Capabilites.
 *
 * It also can combine them from seveal inverters.
 * (planned)
 *  */

class CHTMLWriter: public IDataFilter
{
public:
	CHTMLWriter( const string &name, const string & configurationpath );

	enum Commands
	{
		CMD_INIT
	};

	virtual ~CHTMLWriter();
};

#endif /* CHTMLWRITER_H_ */
