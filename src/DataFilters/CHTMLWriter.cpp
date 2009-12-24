/*
 * CHTMLWriter.cpp
 *
 *  Created on: Dec 20, 2009
 *      Author: tobi
 */

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
