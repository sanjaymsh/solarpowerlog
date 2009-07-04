/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CCSVOutputFilter.cpp), the license terms are:

 You can redistribute it and/or modify it under the terms of the GNU
 General Public License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This programm is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/** \file CCSVOutputFilter.cpp
 *
 *  Created on: Jun 29, 2009
 *      Author: tobi
 */
#include <assert.h>

#include "configuration/Registry.h"
#include "interfaces/CWorkScheduler.h"

#include <libconfig.h>

#include "Inverters/Capabilites.h"
#include "patterns/CValue.h"

#include "CCSVOutputFilter.h"

CCSVOutputFilter::CCSVOutputFilter( const string & name,
	const string & configurationpath ) :
	IDataFilter(name, configurationpath)
{
	// Schedule the initialization and subscriptions later...
	ICommand *cmd = new ICommand(CMD_INIT, this, 0);
	Registry::GetMainScheduler()->ScheduleWork(cmd);

}

CCSVOutputFilter::~CCSVOutputFilter()
{
	// TODO Auto-generated destructor stub
}

bool CCSVOutputFilter::CheckConfig()
{
	string setting;
	string str;
	bool ret = true;

#if 0
	libconfig::Setting & set = Registry::Instance().GetSettingsForObject(
		configurationpath);

	setting = "datasource";
	if (!set.exists(setting) || !set.getType()
		== libconfig::Setting::TypeString) {
		cerr << "Setting " << setting << " in " << configurationpath
			<< "." << name
			<< " missing or of wrong type (wanted a string)"
			<< endl;
		ret = false;
	} else {
		set.lookupValue(setting, str);
		IInverterBase *i = Registry::Instance().GetInverter(str);
		if (!i) {
			cerr << "Setting " << setting << " in "
				<< configurationpath << "." << name
				<< ": Cannot find instance of Inverter with the name "
				<< str << endl;
		}
	}

	setting = "logfile";
	if (!set.exists(setting) || !set.getType()
		== libconfig::Setting::TypeString) {
		cerr << "Setting " << setting << " in " << configurationpath
			<< "." << name
			<< " of wrong type (wanted true or false)" << endl;
		ret = false;
	}

	setting = "data2log";
	if (!set.exists(setting)) {
		cerr << "Setting " << setting << " in " << configurationpath
			<< "." << name << " missing " << endl;
		ret = false;
	} else {



	}

	return ret;
#endif

}

void CCSVOutputFilter::Update( const IObserverSubject *subject )
{
}

void CCSVOutputFilter::ExecuteCommand( const ICommand *cmd )
{
	switch (cmd->getCmd()) {

	case CMD_INIT:
		ICommand *cmd = new ICommand(CMD_CYCLIC, this, 0);
		timespec ts = { 15, 0 };

		CCapability *c = GetConcreteCapability(
			CAPA_INVERTER_QUERYINTERVAL);
		if (c && c->getValue()->GetType() == IValue::float_type) {
			CValue<float> *v = (CValue<float> *) c->getValue();
			ts.tv_sec = v->Get();
			ts.tv_nsec = ((v->Get() - ts.tv_sec) * 1e9);
		}

		break;

	}
}

