/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

 Copyright (C) 2009-2014 Tobias Frost

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ----------------------------------------------------------------------------
 */

/*
 * CDBWriterHelper.cpp
 *
 *  Created on: 05.07.2014
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "porting.h"
#endif

#ifdef  HAVE_FILTER_DBWRITER

#include "CDBWriterHelper.h"

#include "Inverters/Capabilites.h"
#include "Inverters/interfaces/ICapaIterator.h"
#include "patterns/CValue.h"
#include <interfaces/CMutexHelper.h>


#include <assert.h>

CDBWriterHelper::CDBWriterHelper(IInverterBase *base, const ILogger &parent,
    const std::string &table, const std::string &mode, bool logchangedonly,
    float logevery)
{
    logger.Setup(parent.getLoggername(), "CDBWriterHelper(" + table + ")");
    _table = table;
    _table_sanizited = false;
    _logevery = logevery;
    _logchangedonly = logchangedonly;
    _base = base;
    _datavalid = false;

    _olddatastate = NULL;

    if (mode == "continuous") {
        _mode = CDBWriterHelper::continuous;
    } else if (mode == "single") {
        _mode = CDBWriterHelper::single;
    } else if (mode == "cumulative") {
        _mode = CDBWriterHelper::cumulative;
    }

    assert(base);
    CCapability *cap;
    cap = base -> GetConcreteCapability(CAPA_CAPAS_UPDATED);
    assert(cap);
    cap->Subscribe(this);

    cap = base->GetConcreteCapability(CAPA_CAPAS_REMOVEALL);
    assert(cap);
    cap->Subscribe(this);

    cap = base->GetConcreteCapability(CAPA_INVERTER_DATASTATE);
    assert(cap);
    cap->Subscribe(this);
}

CDBWriterHelper::~CDBWriterHelper() {
}

/** Add the tuple Capability, Column to the "should be logged information"
 * Returns "FALSE" if the combination of Capabilty and Column is alreaedy there.
*/
bool CDBWriterHelper::AddDataToLog(const std::string &Capability,
    const std::string &Column)
{

    assert(!Capability.empty());
    assert(!Column.empty());

#warning TODO check capabilities for "special" names and reject if unknown special names are used.

    // Check if we need to complain on the tablename...
    if (!_table_sanizited && !issane(_table)) {
        LOGFATAL(logger, "Tablename is not sane: " << _table);
        return false;
    }
    _table_sanizited = true;

    if (!issane(Column)) {
        LOGFATAL(logger, "Columname is not sane: " << Column);
        return false;
    }

    std::vector<Cdbinfo>::iterator it;
    for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
        if ((*it).Column == Column) {
            LOGFATAL(logger,
                "Table " << _table << ": Column " << Column <<" already used as target:");
            return false;
        }
    }

    LOGDEBUG(logger,
        "\"" << Capability << "\" will be logged to column \"" << Column << "\"");
    _dbinfo.push_back(Cdbinfo(Capability, Column));
    return true;
}

bool CDBWriterHelper::issane(const std::string s)
{
    // needle is selected from mysqli.real-escape-string
    // I added the brackets, which might be overkill...
    const char *needle = "\"\'`[]\0\r\n\x1a%";
    if (std::string::npos != s.find_first_of(needle, 0, sizeof(needle))) {
        return false;
    }
    return true;
}

void CDBWriterHelper::Update(const class IObserverSubject * subject)
{
    assert(subject);
    CCapability *cap = (CCapability *)subject;

    // Datastate changed.
    if (cap->getDescription() == CAPA_INVERTER_DATASTATE) {
        _datavalid = ((CValue<bool> *)cap->getValue())->Get();

#if 0
        // debug code I want to keep for the moment
        // debugs the CValue == and != operator, at least for bool
        if ( ! _olddatastate )
        {
            _olddatastate = cap->getValue()->clone();
        }
        else {
            IValue &o = *cap->getValue();
            IValue &n = *_olddatastate;
            LOGDEBUG(logger, "OLD=" << ((CValue<bool> &)o).Get() <<
                " NEW=" << ((CValue<bool> &)n).Get());
            if ( o == n ) LOGDEBUG(logger, "EQUAL");
            if ( o != n ) LOGDEBUG(logger, "NOT EQUAL");
        }
#endif

        return;
    }

    // Unsubscribe plea -- we do not offer this Capa, our customers will
    // ask our base directly.
    if (cap->getDescription() == CAPA_CAPAS_REMOVEALL) {
        auto_ptr<ICapaIterator> it(_base->GetCapaNewIterator());
        while (it->HasNext()) {
            pair<string, CCapability*> cappair = it->GetNext();
            cap = (cappair).second;
            cap->UnSubscribe(this);
        }
        return;
    }

    //  "caps updated" -- there might be new capabilities...
    // Iterate through all capabilities and if we have customers, subscribe to it

    if (cap->getDescription() == CAPA_CAPAS_UPDATED) {
        auto_ptr<ICapaIterator> it(_base->GetCapaNewIterator());
        while (it->HasNext()) {
            std::string capname;
            std::vector<Cdbinfo>::iterator jt;
            pair<string, CCapability*> cappair = it->GetNext();
            cap = (cappair).second;
            capname = cap->getDescription();
            for (jt = _dbinfo.begin(); jt != _dbinfo.end(); jt++) {
                if ((*jt).Capability == capname) {
                    CMutexAutoLock cma(&mutex);
                    if (!(*jt).previously_subscribed)
                        LOGDEBUG(logger, "Subscribing to " << cap->getDescription());
                    cap->Subscribe(this);
                    (*jt).previously_subscribed = true;
                    break;
                }
            }
        }
        return;
    }

    // OK, some caps has been updated. Lets clone the value :)
    std::vector<Cdbinfo>::iterator it;
    std::string capaname = cap->getDescription();

    for (it = _dbinfo.begin(); it != _dbinfo.end(); it++) {
        if ((*it).Capability == capaname) {
            CMutexAutoLock cma(&mutex);
            if ((*it).LastValue) {
                IValue &last = *(*it).LastValue;
                IValue &now  = *cap->getValue();
                if ( now == last) (*it).wasUpdated = true;
                delete (*it).LastValue;
                (*it).LastValue = NULL;
            }
            (*it).LastValue = cap->getValue()->clone();
            (*it).wasSeen = true;
            break;
        }
    }
}

#endif
