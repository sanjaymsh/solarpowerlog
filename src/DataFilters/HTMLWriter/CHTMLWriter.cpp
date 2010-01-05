/*
 * CHTMLWriter.cpp
 *
 *  Created on: Dec 20, 2009
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef HAVE_FILTER_HTMLWRITER

#include <fstream>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/local_time/local_time.hpp>

#include "DataFilters/HTMLWriter/CHTMLWriter.h"

#include "configuration/Registry.h"
#include "configuration/CConfigHelper.h"
#include "interfaces/CWorkScheduler.h"

#include "Inverters/Capabilites.h"
#include "Inverters/interfaces/ICapaIterator.h"
#include "patterns/CValue.h"
#include "configuration/ILogger.h"

#include "DataFilters/HTMLWriter/formatter/IFormater.h"
#include <string>

/// local helpers: Bundle the cyclic-event calculation here...
/// the following function schedules the next CMD_CYCLIC Command, out of config
/// or out of a Capability
void CHTMLWriter::ScheduleCyclicEvent(enum Commands cmd)
{
	ICommand *ncmd = new ICommand(cmd, this);
	CCapability *c;
	struct timespec ts;

	if (derivetiming) {
		c = GetConcreteCapability(CAPA_INVERTER_QUERYINTERVAL);
		if (c && c->getValue()->GetType() == IValue::float_type) {
			CValue<float> *v = (CValue<float> *) c->getValue();
			ts.tv_sec = v->Get();
			ts.tv_nsec = ((v->Get() - ts.tv_sec) * 1e9);
		} else {
			LOG_INFO(logger,
					"INFO: The associated inverter does not specify the "
					"queryinterval. Defaulting to 300 seconds. Consider specifing writeevery");
			ts.tv_sec = 300;
			ts.tv_nsec = 0;
		}
	} else if (writeevery > 0.0001) {
		ts.tv_sec = writeevery;
		ts.tv_nsec = ((long) (writeevery - ts.tv_sec)) * 1e9;
	} else {
		// writevery 0.
		ts.tv_sec = 300;
		ts.tv_nsec = 0;
	}

	Registry::GetMainScheduler()->ScheduleWork(ncmd, ts);
}

CHTMLWriter::CHTMLWriter(const string & name, const string & configurationpath) :
	IDataFilter(name, configurationpath), updated(false), datavalid(false)
{

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

CHTMLWriter::~CHTMLWriter()
{
	// TODO Auto-generated destructor stub
}

bool CHTMLWriter::CheckConfig()
{
	bool fail = false;
	CConfigHelper hlp(configurationpath);

	// Get writeevery, if not existant, remember that for later extrapolation out
	// of the parents Capability.
	if (hlp.CheckConfig("writeevery", libconfig::Setting::TypeFloat, true)) {
		if (!hlp.GetConfig("writeevery", writeevery, (float) 300.0)) {
			derivetiming = true;
		}
		if (writeevery < 0.0) {
			LOG_ERROR(logger, "Configuration Error: writeevery must be positive.");
			fail = true;
		} else if (writeevery < 0.00001) {
			LOG_ERROR(logger, "Configuration Error: writeevery=0 not yet implemented.");
			fail = true;
		}

	} else {
		fail = true;
	}

	if (hlp.CheckConfig("generate_template", libconfig::Setting::TypeBoolean,
			true) && hlp.CheckConfig("generate_template_dir",
			libconfig::Setting::TypeString, true)) {
		hlp.GetConfig("generate_template", generatetemplate, false);
		hlp.GetConfig("generate_template_dir", generatetemplatedir,
				std::string("/tmp/"));

		if (generatetemplatedir.size()
				&& generatetemplatedir[generatetemplatedir.size() - 1] != '/') {
			generatetemplatedir += '/';
		}
	} else {
		fail = true;
	}

	if (hlp.CheckConfig("htmlfile", libconfig::Setting::TypeString)) {
		hlp.GetConfig("htmlfile", htmlfile);
	} else {
		fail = true;
	}

	if (hlp.CheckConfig("templatefile", libconfig::Setting::TypeString)) {
		hlp.GetConfig("templatefile", templatefile);
	} else {
		fail = true;
	}

	return !fail;
}

void CHTMLWriter::Update(const IObserverSubject *subject)
{
	// note: the subject must be a CCapability here.
	// to avoid neverending casting we do that once.
	CCapability *parentcap = (CCapability *) subject;

	// check for the mandatory Capas now, as they might require
	// immediate actions.
	if (parentcap->getDescription() == CAPA_CAPAS_REMOVEALL) {
		CCapability *ourcap;
		// forward the notification.
		// but -- to be nice -- update the value first
		ourcap = IInverterBase::GetConcreteCapability(CAPA_CAPAS_REMOVEALL);
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
		CCapability *c = IInverterBase::GetConcreteCapability(
				CAPA_CAPAS_UPDATED);
		*(CValue<bool> *) c->getValue()
				= *(CValue<bool> *) parentcap->getValue();
		c->Notify();
		// there are new caps, but currently we won't do anything with it
		// later we want to check the list if we have attach ourself to the
		// listernes.
		return;
	}

	if (parentcap->getDescription() == CAPA_INVERTER_DATASTATE) {
		datavalid = ((CValue<bool> *) parentcap->getValue())->Get();
		return;
	}

	if (!updated) {
		ICommand *cmd = new ICommand(CMD_UPDATED, this);
		Registry::GetMainScheduler()->ScheduleWork(cmd);
	}
	updated = true;

}

void CHTMLWriter::ExecuteCommand(const ICommand *cmd)
{

	switch (cmd->getCmd())
	{

	case CMD_INIT:
	{
		string tmp;
		CConfigHelper cfghlp(configurationpath);
		CCapability *c;

		// Subscribe to this->base inverter, all the required ones...
		if (cfghlp.GetConfig("datasource", tmp)) {
			base = Registry::Instance().GetInverter(tmp);
			if (base) {
				c = base->GetConcreteCapability(CAPA_CAPAS_UPDATED);
				assert(c); // this is required to have....
				c->Subscribe(this);

				c = base->GetConcreteCapability(CAPA_CAPAS_REMOVEALL);
				assert(c);
				c->Subscribe(this);

				c = base->GetConcreteCapability(CAPA_INVERTER_DATASTATE);
				assert(c);
				c->Subscribe(this);
			} else {
				LOG_WARN(logger,
						"Warning: Could not find data source to connect. Filter: "
						<< configurationpath << "." << name);
			}
		}

		ScheduleCyclicEvent(CMD_CYCLIC);
	}
		break;

	case CMD_CYCLIC:
	{

		DoCyclicCmd(cmd);
		ScheduleCyclicEvent(CMD_CYCLIC);
	}

	case CMD_UPDATED:
	{
		// prepared for the next version.
		// "do on update mode"
		break;
	}
	}

}

void CHTMLWriter::DoCyclicCmd(const ICommand *)
{
	ofstream fs;

	// C-Template vars
	TMPL_varlist *tmpl_looplist = NULL; /**< list for the inverter loop */
	TMPL_loop *tmpl_loop = NULL; /**< loop */
	TMPL_varlist *tmpl_list = NULL; /**< list for generationg the out */

	if (!datavalid) {
		return;
	}

	if (generatetemplate) {
		std::string s;
		CConfigHelper cfg(configurationpath);
		cfg.GetConfig("name", s);
		s = generatetemplatedir + s + ".html";
		fs.open(s.c_str(), fstream::out | fstream::trunc | fstream::binary);

#ifdef HAVE_WIN32_API
		if (fs.fail()) {
			fs.clear();
			fs.open(s.c_str(), fstream::out | fstream::app | fstream::binary);
		}
#endif
		if (fs.fail()) {
			LOG_WARN(logger,"Template-Assistant: Failed to open file: " << s);
			fs.close();
		}

		fs << "<html><head></head><body><h1>This are the variables known to "
			"the template system: </h1>" << endl << "<table border=1>"
			"<tr><th>Name of the Attribute</th><th>Current Value</th></tr>";
	}

	// Get the configuration to the formatting specs and map them
	// TODO make this only once....
	multimap<std::string, std::string> formattermap;
	std::string s = configurationpath + ".formatters";
	cerr << s << endl;
	CConfigHelper formatters(s);
	for (int i = 0;; i++) {
		std::string s1, s2;
		if (formatters.GetConfigArray(i, 0, s1) && formatters.GetConfigArray(i,
				1, s2)) {
			formattermap.insert(pair<std::string, std::string> (s1, s2));
			LOG_TRACE(logger, "Inserting formatter spec <" << s1 << ',' << s2 << '>');
		} else {
			break;
		}
	}

	// reminder: this first version only attaches one inverter!
	// Step one:
	// loop over all capabilites and add it to the list

	// Add the number of the inverter to the exported vars, as the snippets
	// probably want to do something special on the first one only.
	// TODO FIXME currently, there is only one. So we make this dynamic later.

	tmpl_looplist = TMPL_add_var(tmpl_looplist, "iteration", "0", NULL);
	if ( generatetemplate ) {
		fs << "<tr><td> iteration </td><td> " << "0" << " </td>\n";
	}

	auto_ptr<ICapaIterator> cit(GetCapaNewIterator());
	while (cit->HasNext()) {
		multimap<std::string, std::string>::iterator it;
		pair<string, CCapability*> cappair = cit->GetNext();

		std::string value = *(cappair.second->getValue());

		for (it = formattermap.find(cappair.first); it != formattermap.end(); it++) {
			IFormater *frmt;
			string x = (*it).second;
			if ((frmt = IFormater::Factory(x, configurationpath))) {
				if (!frmt->Format(value, value)) {
					LOG_ERROR(logger,"Could not reformat " << cappair.first <<
							": Formater reported error.");
				}
				delete frmt;
			} else {
				LOG_ERROR(logger,"Formatter " << x << "unknown" );
			}
		}

		tmpl_looplist = TMPL_add_var(tmpl_looplist, cappair.first.c_str(),
				value.c_str(), NULL);

		if (this->generatetemplate) {
			fs << "<tr><td> " << cappair.first << " </td><td> " << value
					<< " </td>\n";
		}
	}

	// adding the loop entry to the loop varlist.
	tmpl_loop = TMPL_add_varlist(tmpl_loop, tmpl_looplist);
	tmpl_looplist = NULL;

	// (here, we would add the other inverters linked to this story)

	// now adding all the stuff to the main list.
	tmpl_list = TMPL_add_loop(tmpl_list, "inverters", tmpl_loop);

	if (generatetemplate) {
		fs
				<< "<tr><th>HTMWriter Capabilities</th><th>(these are outside of the loop)</th></tr>"
				<< endl;
	}
	map<string, CCapability*>::const_iterator it;
	for (it = CapabilityMap.begin(); it != CapabilityMap.end(); it++) {
		pair<string, CCapability*> cappair = *it;
		std::string tmpstring = *(cappair.second->getValue());

		tmpl_list = TMPL_add_var(tmpl_list, cappair.first.c_str(),
				tmpstring.c_str(), NULL);

		if (this->generatetemplate) {
			fs << "<tr><td> " << cappair.first << " </td><td> " << tmpstring
					<< " </td>\n";
		}

	}

	// template assistance is now done, so close the file.
	if (generatetemplate) {
		fs << "</table></body></html>";
		fs.close();
	}

	// okay, now open a file in memory to catch template errors.
	// (if supported at least)
	FILE *errfile = NULL;
	FILE *out = NULL;
#ifdef HAVE_OPEN_MEMSTREAM
	char *ptr = NULL;
	size_t size = 0;
#endif

#ifdef HAVE_OPEN_MEMSTREAM
	errfile = open_memstream(&ptr, &size);
#else
#warning Note: opem_memstream not available on this platform. Will not be able to tell you template errors
#endif
	// generate filename of the output file
	if (htmlfile.find("%s") != std::string::npos) {
		boost::gregorian::date today(boost::gregorian::day_clock::local_day());
		char buf[htmlfile.size() + 10]; //note: the %s will be removed, so +10 is enough.
		int year = today.year();
		int month = today.month();
		int day = today.day();

		snprintf(buf, sizeof(buf) - 1, "%s%04d-%02d-%02d%s", htmlfile.substr(0,
				htmlfile.find("%s")).c_str(), year, month, day,
				htmlfile.substr(htmlfile.find("%s") + 2, string::npos).c_str());

		out = fopen(buf, "w+");
		if (out == 0) {
			LOG_ERROR(logger, "Could not open filename "<< buf);
		}

	} else {
		out = fopen(htmlfile.c_str(), "w+");
		if (out == NULL) {
			LOG_ERROR(logger, "Could not open filename "<< htmlfile);
		}
	}

	if (out && -1 == TMPL_write(templatefile.c_str(), NULL, NULL, tmpl_list,
			out, errfile)) {
		// error while writing. lets examine errfile.
		// we need first to close the file to update ptr and size.
#ifdef HAVE_OPEN_MEMSTREAM
		fclose(errfile);
		errfile = NULL; // avoid closing a 2nd time later.
		LOG_ERROR(logger, "Error while writing html file."
				" The template library reported this: " << ptr);
#else
		// esp. for the cygwin 1.5 port, we cannot tell the reason why it happened.
		// patches are welcome to open a temporary file on disk instead and then
		// (when cagwin 1.7 comes out, this is no longer an issue: They implemented
		// the GNU extended syscall)
		LOG_ERROR(logger, "Error while writing html file (template error)");
#endif

	} else if (out) {
		LOG_TRACE(logger, "Done writing HTML File. Wrote " << ftell(out) << " Bytes");
	}

	// cleanup.
	if (tmpl_list)
		TMPL_free_varlist(tmpl_list);
	if (out)
		fclose(out);
#ifdef HAVE_OPEN_MEMSTREAM
	if (errfile)
		fclose(errfile);
	if (ptr)
		free(ptr);
#endif
}

void CHTMLWriter::CheckOrUnSubscribe(bool subscribe)
{
	if (!base)
		return;

	CCapability *cap = base->GetConcreteCapability(CAPA_INVERTER_DATASTATE);
	if (cap)
		cap->SetSubscription(this, subscribe);

}

#endif
