/*
 * CHTMLWriter.h
 *
 *  Created on: Dec 20, 2009
 *      Author: tobi
 */

#ifndef CHTMLWRITER_H_
#define CHTMLWRITER_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_FILTER_HTMLWRITER

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
	CHTMLWriter(const string &name, const string & configurationpath);

	virtual ~CHTMLWriter();

	virtual bool CheckConfig();

	virtual void Update(const IObserverSubject *subject);

	virtual void ExecuteCommand(const ICommand *cmd);

	enum Commands
	{
		CMD_INIT,
		CMD_UPDATED,
		CMD_CYCLIC,
	};


private:

	void CheckOrUnSubscribe( bool subscribe = true );

	// Configuraion cache
	float writeevery;
	bool derivetiming;
	bool generatetemplate;
	std::string generatetemplatedir;
	std::string htmlfile;
	std::string templatefile;

	bool updated;
	bool datavalid;

	/// Helper: Generated the cyclic ICommand cmd and attach it.
	void ScheduleCyclicEvent(enum CHTMLWriter::Commands cmd);

	/// DoCyclicCmd -- makes the page.
	void DoCyclicCmd(const ICommand *);

};

#endif

#endif /* CHTMLWRITER_H_ */
