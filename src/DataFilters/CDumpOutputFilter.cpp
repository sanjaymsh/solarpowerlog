/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CDumpOutputFilter.cpp), the license terms are:

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

/** \file CDumpOutputFilter.cpp
 *
 *  Created on: Jun 1, 2009
 *      Author: tobi
 */

#include <assert.h>

#include "CDumpOutputFilter.h"
#include "DataFilters/interfaces/IDataFilter.h"
#include "configuration/Registry.h"

#include "Inverters/Capabilites.h"
#include "patterns/ICommand.h"
#include "configuration/Registry.h"
#include "interfaces/CWorkScheduler.h"
#include "Inverters/interfaces/ICapaIterator.h"
#include "patterns/CValue.h"

CDumpOutputFilter::CDumpOutputFilter( const string &name,
	const string & configurationpath ) :
	IDataFilter(name, configurationpath)
{
	string tmp;
	libconfig::Setting &set = Registry::Instance().GetSettingsForObject(
		configurationpath);

	// Get our data source (Observer)
	string setting = "datasource";
	// char *tmp;
	if (set.lookupValue("datasource", tmp)) {
		base = Registry::Instance().GetInverter(tmp);
		if (base) {
			CCapability *cap = base->GetConcreteCapability(
				CAPA_CAPAS_UPDATED);
			assert(cap); // this is required to have....
			cap->Subscribe(this);

			cap = base->GetConcreteCapability(CAPA_CAPAS_REMOVEALL);
			assert(cap);
			cap->Subscribe(this);

			cap = base->GetConcreteCapability(
				CAPA_INVERTER_DATASTATE);
			assert(cap);
			cap->Subscribe(this);
		} else {
			cerr
				<< "Warning: Could not find data source to connect. Filter: "
				<< configurationpath << "." << name << endl;
		}

		// Schedule the initialization and subscriptions later...
		ICommand *cmd = new ICommand(CMD_INIT, this, 0);
		Registry::GetMainScheduler()->ScheduleWork(cmd);

	}
}

CDumpOutputFilter::~CDumpOutputFilter()
{
	if (base) {
		ICapaIterator *it = base->GetCapaNewIterator();
		pair<string, CCapability*> pair;
		while (it->HasNext()) {
			pair = it->GetNext();
			pair.second->UnSubscribe(this);
		}
	}
}

bool CDumpOutputFilter::CheckConfig()
{
	string setting;
	string str;
	bool ret = true;

	libconfig::Setting &set = Registry::Instance().GetSettingsForObject(
		configurationpath);

	setting = "datasource";
	if (!set.exists(setting) || !set.getType()
		== libconfig::Setting::TypeString) {
		cerr << "Setting " << setting << " in " << configurationpath
			<< "." << name
			<< " missing or of wrong type (wanted a string)"
			<< endl;
		ret = false;
	}
	return ret;
}

void CDumpOutputFilter::Update( const IObserverSubject *subject )
{
	assert (subject);

	// note: the subject must be a CCapability here.
	// to avoid neverending casting we do that once.
	CCapability *parentcap = (CCapability *) subject;
	CCapability *ourcap;

	// check for the mandatory Capas now, as they might require
	// immediate actions.

	if (parentcap->getDescription() == CAPA_CAPAS_REMOVEALL) {
		// forward the notification.
		// but -- to be nice -- update the value first
		ourcap = GetConcreteCapability(CAPA_CAPAS_REMOVEALL);
		assert (ourcap);
		assert (ourcap->getValue()->GetType() == CAPA_CAPAS_REMOVEALL_TYPE);
		assert (parentcap->getValue()->GetType() == CAPA_CAPAS_REMOVEALL_TYPE);

		CValue<bool> *a, *b;
		a = (CValue<bool> *) (ourcap->getValue());
		b = (CValue<bool> *) (parentcap->getValue());
		*a = *b;
		ourcap->Notify();

		CheckOrUnSubscribe(false);
		return;
	}

	if (parentcap->getDescription() == CAPA_CAPAS_UPDATED) {
		// this one will be dereferred, but only if we do
		// not have one pending.
		if (AddedCaps)
			return;
		AddedCaps = true;
		ICommand *cmd = new ICommand(CMD_ADDED_CAPAS, this, 0);
		Registry::GetMainScheduler()->ScheduleWork(cmd);
		return;
	}

	// All others does not need to handled here, its ok in the cyclic.

}

void CDumpOutputFilter::ExecuteCommand( const ICommand *cmd )
{

	switch (cmd->getCmd()) {
	case CMD_INIT:
		CheckOrUnSubscribe(true);
		// falling through
	case CMD_CYCLIC:
	{
		ICommand *cmd = new ICommand(CMD_CYCLIC, this, 0);
		timespec ts = { 1, 0 };
		Registry::GetMainScheduler()->ScheduleWork(cmd, ts);
		DoCyclicWork();
		break;
	}

	case CMD_UNSUBSCRIBE:
		CheckOrUnSubscribe(false);
		break;

	case CMD_ADDED_CAPAS:
		cout << name << ":" << configurationpath
			<< " New Capability(ies) reported" << endl;
		AddedCaps = false;
		CheckOrUnSubscribe(true);
		GetConcreteCapability(CAPA_CAPAS_UPDATED)->Notify();
		break;

	}
}

// On serveral updates, we have to go through all subscriptions
// and check, if we have to subscribe to any of them
// The parameter sets, if we subscribe or unsubscribe.
void CDumpOutputFilter::CheckOrUnSubscribe( bool subscribe )
{
	if (!base)
		return;

	CCapability *cap = base->GetConcreteCapability(
		CAPA_INVERTER_MANUFACTOR_NAME);
	if (cap && !cap->CheckSubscription(this))
		cap->SetSubsubscription(this, subscribe);

	cap = base->GetConcreteCapability(CAPA_INVERTER_MODEL);
	if (cap && !cap->CheckSubscription(this))
		cap->SetSubsubscription(this, subscribe);

	cap = base->GetConcreteCapability(CAPA_INVERTER_ACPOWER_TOTAL);
	if (cap && !cap->CheckSubscription(this))
		cap->SetSubsubscription(this, subscribe);

	cap = base->GetConcreteCapability(CAPA_INVERTER_KWH_Y2D);
	if (cap && !cap->CheckSubscription(this))
		cap->SetSubsubscription(this, subscribe);

	cap = base->GetConcreteCapability(CAPA_INVERTER_KWH_M2D);
	if (cap && !cap->CheckSubscription(this))
		cap->SetSubsubscription(this, subscribe);

	cap = base->GetConcreteCapability(CAPA_INVERTER_KWH_M2D);
	if (cap && !cap->CheckSubscription(this))
		cap->SetSubsubscription(this, subscribe);
}

// Check if we've overriden the capability and return our or the parents one.
CCapability *CDumpOutputFilter::GetConcreteCapability(
	const string & identifier )
{
	CCapability *c;
	if ((c = IInverterBase::GetConcreteCapability(identifier)))
		return c;
	else
		return base->GetConcreteCapability(identifier);
}

/// This simple Data-Dumper will just dump all over all capas it has.
void CDumpOutputFilter::DoCyclicWork( void )
{
	cout << configurationpath << "." << name << " Own Capabilities:"
		<< endl << endl;
	map<string, CCapability*>::iterator it = GetCapabilityIterator();
	while (it != GetCapabilityLastIterator()) {
		cout << (*it).first << ' ';
		for (int i = (*it).first.length() + 1; i < 30; i++)
			cout << '.';
		cout << DumpValue((*it).second->getValue()) << endl;
	}

	cout << configurationpath << "." << name << " Derived Capabilities:"
		<< endl << endl;
	ICapaIterator *cit = GetCapaNewIterator();
	while (cit->HasNext()) {
		pair<string, CCapability*> cappair = cit->GetNext();
		cout << (cappair).first << ' ';
		for (int i = (cappair).first.length() + 1; i < 30; i++)
			cout << '.';
		cout << DumpValue((*it).second->getValue()) << endl;
	}

	delete cit;
	cout << endl;

}

string CDumpOutputFilter::DumpValue( IValue *value )
{
	enum IValue::factory_types type = value->GetType();
	string ret;

	switch (type) {
	case IValue::bool_type:
		ret = ((CValue<bool> *) value)->Get() ? "true/on/active"
			: "false/off/inactive";
		break;

	case IValue::float_type:
		ret = ((CValue<float>*) value) -> Get();
		break;

	case IValue::int_type:
		ret = ((CValue<int>*) value) -> Get();
		break;

	case IValue::string_type:
		ret = ((CValue<string>*) value) -> Get();
		break;
	}
	return ret;
}
