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

/*
 * CSputnikCommand.h
 *
 *  Created on: 14.05.2012
 *      Author: tobi
 */

#ifndef CSPUTNIKCOMMAND_H_
#define CSPUTNIKCOMMAND_H_

#include <string>
#include <vector>
#include <stdlib.h>

#include "Inverters/interfaces/InverterBase.h"
#include "patterns/CValue.h"
#include "Inverters/Capabilites.h"

/* Defines and handles a single commmand the Sputnik Inverter.
 *
 * CSputnikCommand (CSC) will in the long term replace the handling code in
 * CSputnikInverter for an object orientated approach.
 *
 * The object will store all knowledge about an individual command, allows the
 * parsing and eventually will also take over the Capabilities handling.
 *
 * It will also allow an strategy to wisely let the command itself decide when it
 * should be issued and how often, and to probe if an actual command is indeed supported
 * by the inverter.
 *
 * Milestones to implement functionality:
 *  1- CSC will store information about
 *      - (implemented) command token, scaling information, expected telegram length,
 *      - (implemented for basic types) do the parsing and return the parsed result.
 *
 *  2- CSC will keep track if a command is supported by the Inverter and cease issuing command if not.
 *  3- (Specialisations of CSCs might handle for some commands more often than others.)
 *  4- CSC will take the Capabilities handling: creating the Capabilities, updating the values, notfiing subscribers
 *
 *  Strategy:
 *  - CSCs are kept in a list attached to the Inverters Object
 *  - The CSCS are instanciated on construction time of the Inverter's object with all the information they need to operate.
 *  5- CSC will be registered by the Inverter at Instanciation time by key parameters-

 *
 *
 * */

/** Inteface Class for the Commands.
 *
 * Implements also the common storage and methods */
template <class T>
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

    /** Returns a const reference to the token string to be used for the communication
     * when requesting the data from the inverter.
     * (note: can be overriden for complex datas, for example if more than one command is
     * required to get thw whole set. */
    virtual const std::string& GetCommand(void) {
        return command;
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

protected:
    /** Update / Register capability, if needed */
    virtual void CapabilityHandling(T value) {
        assert(inverter);
        CCapability *cap = inverter->GetConcreteCapability(capaname);
# warning this code will only work / needs to be updated safely when it ensured \
     that the Capability is not created elsewhere, especially with another type. \
     It also breaks the way the IValue interface is currently used, as the type information \
        is not stored with the IValue.\
        note2: http://ciaranm.wordpress.com/2010/05/24/runtime-type-checking-in-c-without-rtti/

        if (!cap) {
           IValue *v = new CValue<T>;
           ((CValue<T>*)v)->Set(value);
           cap = new CCapability(capaname,v,inverter);
           inverter->GetConcreteCapability(CAPA_CAPAS_UPDATED)->Notify();
           return;
        }

        // here the type check has to go... after conversion of IValue to boost::any it will throw
        // on type mismatch.
        CValue<T> *v = (CValue<T> *)cap->getValue();
        v->Set(value);
        cap->Notify();
    };

protected:
    std::string command;
    std::string capaname;
    int max_answer_len;
    IInverterBase *inverter;

};





/** Command Class template for all basic types with the support to scale.*/
template<class T>
class CSputnikCommand : public ISputnikCommand<T>
{
public:

    CSputnikCommand(const std::string &cmd, int max_answer_len, T scale,
        IInverterBase *inv, const std::string & capname)
        : scale(scale)
    {
        ISputnikCommand<T>::command = cmd;
        ISputnikCommand<T>::max_answer_len = max_answer_len;
        ISputnikCommand<T>::inverter = inv;
        ISputnikCommand<T>::capaname = capname;
    }

    unsigned long convertlong(const std::vector<std::string> & tokens) {
        return strtoul(tokens[1].c_str(), NULL, 16);
    }

    T convert(const std::vector<std::string> & tokens) {
        unsigned long integer = this->convert(tokens);
        T result = integer;
        result *= scale;
        return result;
    }

private:
    T scale;
};

#if 0
/** Special implementation for the Inverter's Firmware proptery...
 * As it is built from two commands.
 */
class CSputnikCommandSoftwareVersion : public ISputnikCommand
{
    CSputnikCommandSoftwareVersion(const std::string &command,
        int max_answer_len)
        : ISputnikCommand(command, max_answer_len) {
    }

private:


};
#endif

#endif /* CSPUTNIKCOMMAND_H_ */
