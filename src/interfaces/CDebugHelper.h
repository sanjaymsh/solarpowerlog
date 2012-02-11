/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (CDebugHelper.h), the license terms are:

 You can redistribute it and/or  modify it under the terms of the GNU Lesser
 General Public License (LGPL) as published by the Free Software Foundation;
 either version 3 of the License, or (at your option) any later version.

 This programm is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/** \file CDebugHelper.h
 *
 * Class to collect internal runtime data helpful for debugging.
 *
 * DbugHelper is a runtime-debug-data storage to  registers runtime data from the
 * components and offers a interface to dump this inforation to cerr.
 *
 * \section cdebughelperusage Usage
 *
 * - Register your <pair
 *
 * \warning this is debugging only!
 *  Created on: 30.12.2011
 *      Author: tobi
 */

#ifndef CDEBUGHELPER_H_
#define CDEBUGHELPER_H_

#include <list>
#include <string>
#include <iostream>

/** \fixme COMMENT ME
 *
 *
 * TODO DOCUMENT ME!
 */
/** \fixme COMMENT ME
 *
 *
 * TODO DOCUMENT ME!
 */

class IDebugObject
{
public:
    IDebugObject() {};
    virtual ~IDebugObject();
    virtual void Dump(void) = 0;


};

template<class T>
class CDebugObject : public IDebugObject
{
public:
    virtual ~CDebugObject() {};

    CDebugObject(std::string &identifier, T &v) {
        id = identifier;
        value = &v;
    }

    CDebugObject(const char *identifier, T &v) {
        id = identifier;
        value = &v;
    }

    CDebugObject(const char *identifier, void *v) {
        id = identifier;
        value = &v;
    }

    virtual void Dump(void) {
        std::cerr << id << "=" << *value << std::endl;
    }

    std::string id;
    T *value;

};

class CDebugHelperCollection
{
public:
    virtual ~CDebugHelperCollection();

    CDebugHelperCollection(std::string &context) {
        this->context = context;
        this->registered = false; // not yet registered at Registry
    }

    CDebugHelperCollection(const char *context) {
        this->context = context;
        this->registered = false; // not yet registered at Registry
    }

    void Register(IDebugObject *dobj);
    void Unregister(IDebugObject *dobj);
    void Dump();

private:
    bool registered;
    std::string context;
    std::list<IDebugObject *> dobjlist;

};

#endif /* CDEBUGHELPER_H_ */
