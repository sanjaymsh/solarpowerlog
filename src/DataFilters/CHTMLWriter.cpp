/*
 * CHTMLWriter.cpp
 *
 *  Created on: Dec 20, 2009
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_FILTER_HTMLWRITER

#include "DataFilters/CHTMLWriter.h"

#include "configuration/Registry.h"
#include "configuration/CConfigHelper.h"
#include "interfaces/CWorkScheduler.h"

CHTMLWriter::CHTMLWriter(const string & name, const string & configurationpath)
: 	IDataFilter(name, configurationpath)
{

	// Schedule the initialization and subscriptions later...
	ICommand *cmd = new ICommand(CMD_INIT, this);
	Registry::GetMainScheduler()->ScheduleWork(cmd);

}

CHTMLWriter::~CHTMLWriter()
{
	// TODO Auto-generated destructor stub
}


bool CHTMLWriter::CheckConfig()
{
	cout << "The module cannot be used yet";
	return false;
}



void CHTMLWriter::Update(const IObserverSubject *subject)
{

}



#endif
