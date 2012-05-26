/* ----------------------------------------------------------------------------
 solarpowerlog -- photovoltaic data logging

 Copyright (C) 2009-2012 Tobias Frost

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "patterns/CValue.h"
#include "Inverters/Capabilites.h"
#include "Inverters/interfaces/InverterBase.h"
/*
 * ISputnikCommand.h
 *
 *  Created on: 19.05.2012
 *      Author: tobi
 */

#ifndef ISPUTNIKCOMMAND_H_
#define ISPUTNIKCOMMAND_H_

/** Interface Class for the Commands.
 *
 * Implements also the common storage and methods */
class ISputnikCommand
{

public:

    ISputnikCommand() {
    }

    ISputnikCommand(const std::string &command,
        int max_answer_len , IInverterBase *inv, const std::string & capaname)
        : command(command), max_answer_len(max_answer_len),inverter(inv), capaname(capaname) {
    }

    virtual ~ISputnikCommand() {
    }

    /** Returnes the maximium expected length (usually set at compile time) */
    virtual int GetMaxAnswerLen(void) {
        return max_answer_len;
    }
    /** Setter for the Max Answer Len, if needed*/
    virtual void SetMaxAnswerLen(int max) {
        max_answer_len = max;
    }

    /** Should we consider this command to be issued?
     *
     * \returns true if so, else do not issue command at this time. */
    virtual bool ConsiderCommand() {
        return true;
    }

    /** Returns a const reference to the token string to be used for the communication
     * when requesting the data from the inverter.
     * (note: can be overriden for complex datas, for example if more than one command is
     * required to get thw whole set. */
    virtual const std::string& GetCommand(void) {
        return command;
    }

    /** Helper: Return the length of the command to be issued. */
    virtual unsigned int GetCommandLen(void) {
        return command.length();
    }

    /** Check if the token is handled by this instance.
     * \returns true if it is, else false
     *
     * For complex data, which is assembled from more than one command, this
     * needs be overridden.
    */
    virtual bool IsHandled(const std::string &token) {
        return (token == command);
    }

    /// handles the parsing, and handles the capability then.
    /// must bei implemented in the derived class.
    virtual void handle_token(const std::vector<std::string> & tokens) {};

protected:
    /** Update / Register capability, if needed */

    template <class T>
    void CapabilityHandling(T value) throw() {
        assert(inverter);
        CCapability *cap = inverter->GetConcreteCapability(capaname);

        if (!cap) {
           IValue *v = new CValue<T>;
           ((CValue<T>*)v)->Set(value);
           cap = new CCapability(capaname,v,inverter);
           inverter->GetConcreteCapability(CAPA_CAPAS_UPDATED)->Notify();
           return;
        }

        // Check for the type and throw an exception if not equal.
        // (as capabilites are created by this class, this should be not hapen)
         if ( CValue<T>::IsType(cap->getValue())) {
            CValue<T> *v = (CValue<T> *)cap->getValue();
            v->Set(value);
            cap->Notify();
            return;
        } else {
            std::bad_cast e;
            LOGERROR(inverter->logger,"Bad cast for command " + command);
            throw e;
        }
    };

    /// Alternative implementation to sepcify also the capability to be updated.
    /// (For example if more than one capability is attached to this command)
    template <class T>
    void CapabilityHandling(T value, std::string capname) throw() {
        assert(inverter);
        CCapability *cap = inverter->GetConcreteCapability(capname);

        if (!cap) {
           IValue *v = new CValue<T>;
           ((CValue<T>*)v)->Set(value);
           cap = new CCapability(capname,v,inverter);
           inverter->GetConcreteCapability(CAPA_CAPAS_UPDATED)->Notify();
           return;
        }

        // Check for the type and throw an exception if not equal.
        // (as capabilites are created by this class, this should be not hapen)
         if ( CValue<T>::IsType(cap->getValue())) {
            CValue<T> *v = (CValue<T> *)cap->getValue();
            v->Set(value);
            cap->Notify();
            return;
        } else {
            std::bad_cast e;
            LOGERROR(inverter->logger,"Bad cast for command " + command);
            throw e;
        }
    };

protected:
    std::string command;
    int max_answer_len;
    IInverterBase *inverter;
    std::string capaname;
};

#endif /* ISPUTNIKCOMMAND_H_ */
