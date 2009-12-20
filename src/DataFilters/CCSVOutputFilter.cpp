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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef  HAVE_FILTER_CSVDUMP

#include <assert.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <memory>

#include <boost/date_time/gregorian/gregorian.hpp>
#include "boost/date_time/local_time/local_time.hpp"

#include "configuration/Registry.h"
#include "configuration/CConfigHelper.h"
#include "interfaces/CWorkScheduler.h"

#include "Inverters/Capabilites.h"
#include "patterns/CValue.h"

#include "CCSVOutputFilter.h"

#include "Inverters/interfaces/ICapaIterator.h"

using namespace std;
using namespace libconfig;
using namespace boost::gregorian;

CCSVOutputFilter::CCSVOutputFilter( const string & name,
	const string & configurationpath ) :
	IDataFilter(name, configurationpath)
{
	headerwritten = false;

	// Schedule the initialization and subscriptions later...
	ICommand *cmd = new ICommand(CMD_INIT, this);
	Registry::GetMainScheduler()->ScheduleWork(cmd);

	// We do not anything on these capabilities, so we remove our list.
	// any cascaded filter will automatically use the parents one...
	CCapability *c = IInverterBase::GetConcreteCapability(
		CAPA_INVERTER_DATASTATE);
	CapabilityMap.erase(CAPA_INVERTER_DATASTATE);
	delete c;

	// Also we wont fiddle with the caps requiring our listeners to unsubscribe.
	// They also should get that info from our base.
	c = IInverterBase::GetConcreteCapability(CAPA_CAPAS_REMOVEALL);
	CapabilityMap.erase(CAPA_CAPAS_REMOVEALL);
	delete c;

}

CCSVOutputFilter::~CCSVOutputFilter()
{
	if (file.is_open())
		file.close();
	// TODO Auto-generated destructor stub
}

bool CCSVOutputFilter::CheckConfig()
{
	string setting;
	string str;

	bool fail = false;

	CConfigHelper hlp(configurationpath);
	fail |= !hlp.CheckConfig("datasource", Setting::TypeString);
	fail |= !hlp.CheckConfig("clearscreen", Setting::TypeBoolean, true);
	fail |= !hlp.CheckConfig("logfile", Setting::TypeString);

	if (hlp.CheckConfig("data2log", Setting::TypeString, false, false)) {
		hlp.GetConfig("data2log", setting);
		if (setting != "all") {
			LOG_ERROR(logger, "Configuration Error: data2log must be \"all\" or of Type Array.");
			fail = true;
		}
	} else if (!hlp.CheckConfig("data2log", Setting::TypeArray)) {
		fail = true;
	}

	// FIXME: This will not work, if the inverter / logger is instanciated later.
	// in the config file. This is not nice and can be avoided.
	// (delegate the to CMD_INIT)
	hlp.GetConfig("datasource", str);
	IInverterBase *i = Registry::Instance().GetInverter(str);
	if (!i) {
		LOG_ERROR(logger,
			"Setting " << setting << " in " << configurationpath
			<< "." << name
			<< ": Cannot find instance of Inverter with the name "
			<< str);
		fail = true;
	}
	return !fail;
}

void CCSVOutputFilter::Update( const IObserverSubject *subject )
{
	assert (subject);
	CCapability *c, *cap = (CCapability *) subject;

	// Datastate changed.
	if (cap->getDescription() == CAPA_INVERTER_DATASTATE) {
		this->datavalid = ((CValue<bool> *) cap->getValue())->Get();
		return;
	}

	// Unsubscribe plea -- we do not offer this Capa, our customers will
	// ask our base directly.
	if (cap->getDescription() == CAPA_CAPAS_REMOVEALL) {
		auto_ptr<ICapaIterator> it(base->GetCapaNewIterator());
		while (it->HasNext()) {
			pair<string, CCapability*> cappair = it->GetNext();
			cap = (cappair).second;
			cap->UnSubscribe(this);
		}
		return;
	}

	// propagate "caps updated"
	if (cap->getDescription() == CAPA_CAPAS_UPDATED) {
		c = GetConcreteCapability(CAPA_CAPAS_UPDATED);
		*(CValue<bool> *) c->getValue()
			= *(CValue<bool> *) cap->getValue();
		c->Notify();
		capsupdated = true;
		return;
	}

}

void CCSVOutputFilter::ExecuteCommand( const ICommand *cmd )
{
	switch (cmd->getCmd()) {

	case CMD_INIT:
	{
		DoINITCmd(cmd);

		ICommand *ncmd = new ICommand(CMD_CYCLIC, this);
		struct timespec ts;
		// Set cyclic timer to the query interval.
		ncmd = new ICommand(CMD_CYCLIC, this);
		ts.tv_sec = 5;
		ts.tv_nsec = 0;

		CCapability *c = GetConcreteCapability(
			CAPA_INVERTER_QUERYINTERVAL);
		if (c && c->getValue()->GetType() == IValue::float_type) {
			CValue<float> *v = (CValue<float> *) c->getValue();
			ts.tv_sec = v->Get();
			ts.tv_nsec = ((v->Get() - ts.tv_sec) * 1e9);
		} else {
			LOG_INFO(logger,
				"INFO: The associated inverter does not specify the "
				"queryinterval. Defaulting to 5 seconds");
		}

		Registry::GetMainScheduler()->ScheduleWork(ncmd, ts);

	}
		break;

	case CMD_CYCLIC:
	{
		DoCYCLICmd(cmd);

		// Set cyclic timer to the query interval.
		ICommand *ncmd = new ICommand(CMD_CYCLIC, this);
		struct timespec ts;
		ts.tv_sec = 5;
		ts.tv_nsec = 0;

		CCapability *c = GetConcreteCapability(
			CAPA_INVERTER_QUERYINTERVAL);
		if (c && c->getValue()->GetType() == IValue::float_type) {
			CValue<float> *v = (CValue<float> *) c->getValue();
			ts.tv_sec = v->Get();
			ts.tv_nsec = ((v->Get() - ts.tv_sec) * 1e9);
		}

		Registry::GetMainScheduler()->ScheduleWork(ncmd, ts);

	}
		break;

	case CMD_ROTATE:
		DoINITCmd(cmd);
		break;

	}
}

void CCSVOutputFilter::DoINITCmd( const ICommand * )
{
	// Do init
	string tmp;
	CConfigHelper cfghlp(configurationpath);
	// Config is already checked (exists, type ok)
	cfghlp.GetConfig("datasource", tmp);

	base = Registry::Instance().GetInverter(tmp);
	if (base) {
		CCapability *cap = base->GetConcreteCapability(
			CAPA_CAPAS_UPDATED);
		assert(cap); // this is required to have....
		if (!cap->CheckSubscription(this))
			cap->Subscribe(this);

		cap = base->GetConcreteCapability(CAPA_CAPAS_REMOVEALL);
		assert(cap);
		if (!cap->CheckSubscription(this))
			cap->Subscribe(this);

		cap = base->GetConcreteCapability(CAPA_INVERTER_DATASTATE);
		assert(cap);
		if (!cap->CheckSubscription(this))
			cap->Subscribe(this);
	} else {
		LOG_ERROR(logger, "Could not find data source to connect. Filter: "
			<< configurationpath << "." << name );
		abort();
	}

	// Try to open the file
	if (file.is_open()) {
		file.close();
	}

	cfghlp.GetConfig("logfile", tmp);
	bool rotate;
	cfghlp.GetConfig("rotate", rotate, false);
	if (rotate) {
		date today(day_clock::local_day());
		//note: the %s will be removed, so +10 is enough.
		char buf[tmp.size() + 10];
		int year = today.year();
		int month = today.month();
		int day = today.day();

		snprintf(buf, sizeof(buf) - 1, "%s%04d-%02d-%02d%s",
			tmp.substr(0, tmp.find("%s")).c_str(), year, month,
			day,
			tmp.substr(tmp.find("%s") + 2, string::npos).c_str());

		tmp = buf;
	}

	// Open the file. We use binary mode, as we want end the line ourself (LF+CR)
	// leaned on RFC4180
	file.clear(); // clear errorstates of fstream.
	file.open(tmp.c_str(), fstream::out | fstream::in | fstream::app
		| fstream::binary);

#ifdef HAVE_WIN32_API
	if (file.fail()) {
		file.clear();
		file.open(tmp.c_str(), fstream::out | fstream::app | fstream::binary);
	}
#endif
	if (file.fail()) {
		LOG_WARN(logger,"Failed to open file" << tmp <<". Logger " << name
			<< " will not work. " );
		file.close();
	}

	headerwritten = false;

	// Set a timer to some seconds after midnight, to enforce rotating with correct date
	boost::posix_time::ptime n =
		boost::posix_time::second_clock::local_time();

	date d = n.date() + days(1);
	boost::posix_time::ptime tomorrow(d);
	boost::posix_time::time_duration remaining = tomorrow - n;

	struct timespec ts;
	ts.tv_sec = remaining.hours() * 3600UL + remaining.minutes() * 60
		+ remaining.seconds() + 10;
	ts.tv_nsec = 0;
	ICommand *ncmd = new ICommand(CMD_ROTATE, this);
	Registry::GetMainScheduler()->ScheduleWork(ncmd, ts);

}

void CCSVOutputFilter::DoCYCLICmd( const ICommand * )
{
	bool compact_file, flush_after_write;
	std::string format;

	CConfigHelper cfg(configurationpath);
#warning document me: Config option
	cfg.GetConfig("Format_Timestamp", format, std::string("%Y-%m-%d %T"));
	#warning document me: config Uption // FIXME
	cfg.GetConfig("Compact_CSV", compact_file, false);
#warning document me: config Uption // FIXME
cfg.GetConfig("flush_file_buffer_immediatly", flush_after_write, true);

	// TODO REMOVE DEBUG CODE (flushing for debugging)

	/* Check for data validty. */
	if (!datavalid) {
		return;
	}

	/* check if CSV-Header needs to be re-emitted.*/
	if (capsupdated || !headerwritten) {
		capsupdated = false;
		if (CMDCyclic_CheckCapas()) {
			headerwritten = false;
		}
	}

	/* check if file is ready */
	if (!file.is_open()) {
		return;
	}

	/* output CSV Header*/
	if (!headerwritten) {
		last_line.clear();
		bool first = true;
		list<string>::const_iterator it;
		for (it = CSVCapas.begin(); it != CSVCapas.end(); it++) {
			if (!first) {
				file << ",";
			} else {
				file << "Timestamp ,";
			}
			first = false;
			file << '\"' << *(it) << '\"';
		}
		// CSV after RFC 4180 requires CR LF
		file << (char) 0x0d << (char) 0x0a;
		headerwritten = true;
	}

	/* finally, output data. */

	// make timestamp
	boost::posix_time::ptime n =
		boost::posix_time::second_clock::local_time();

	// assign facet only to a temporay stringstream.
	// this avoids having a persistent object.
	/// time_facet for the formating of the string
	boost::posix_time::time_facet *facet = new boost::posix_time::time_facet(format.c_str());
	std::stringstream ss;
	ss.imbue(std::locale(ss.getloc(), facet));
	ss << n;
	file << ss.str();
	ss.str("");

	// note: do not delete the facet. This is done by the locale.
	// See: http://rhubbarb.wordpress.com/2009/10/17/boost-datetime-locales-and-facets/
	// (the locale will delete the object, so there is no leak. If we would
	// delete, this crashes.)

	list<string>::const_iterator it;
	CCapability *c;
	IValue *v;
	for (it = CSVCapas.begin(); it != CSVCapas.end(); it++) {
		ss << ",";
		c = base->GetConcreteCapability(*it);
		if (c) {
			v = c->getValue();
			string tmp = (string) *v;

			if (string::npos != tmp.find('"')) {
				string t2 = tmp;
				size_t t;
				while (string::npos != (t = t2.find('"'))) {
					tmp = t2.substr(0, t);
					tmp += '"';
					t2 = t2.substr(t, string::npos);
				}
				tmp += t2;
			}

			if (string::npos != tmp.find(',') || string::npos
				!= tmp.find("\x0d\x0a")) {
				ss << '"' << tmp << '"';
			} else {
				ss << tmp;
			}

		} else {
			// file << ' ';
		}
	}

	if ( !compact_file ||  ss.str() != last_line) {
		file << ss.str() << (char) 0x0d << (char) 0x0a;
		last_line = ss.str();
		if (flush_after_write)
			file << flush;
	}

}

bool CCSVOutputFilter::CMDCyclic_CheckCapas( void )
{
	// get the array
	CConfigHelper cfghlp(configurationpath);
	bool store_all = false;
	bool ret = false;
	string tmp;

	if (cfghlp.GetConfig("data2log", tmp) && tmp == "all") {
		store_all = true;
	}

	if (!store_all) {
		int i = 0;
		while (cfghlp.GetConfigArray("data2log", i++, tmp)) {
			if (search_list(tmp)) {
				continue;
			}
			CSVCapas.push_back(tmp);
			ret = true;
		}

		return ret;
	}

	/** check for new capabilites not already in the list.
	 * Add the new ones to the end of the list. */
	auto_ptr<ICapaIterator> it(base->GetCapaNewIterator());
	pair<string, CCapability*> pair;
	while (it->HasNext()) {
		pair = it->GetNext();
		if (search_list(pair.first)) {
			continue;
		}
		CSVCapas.push_back(pair.first);
		ret = true;
	}
	return ret;

}

bool CCSVOutputFilter::search_list( const string id ) const
{
	list<string>::const_iterator it;
	for (it = CSVCapas.begin(); it != CSVCapas.end(); it++) {
		if (*it == id)
			return true;
	}
	return false;
}

#endif
