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
#include "Inverters/SputnikEngineering/SputnikCommand/BackoffStrategies/ISputnikCommandBackoffStrategy.h"

/**
 * \file ISputnikCommand.h
 *
 *  Abstracts the defintion and handling of a single commmand for the Sputnik Inverter.
 *
 *  The past implementation had a function for every command the inverter
 *  understands, consisting some thousand lines of basically idenetical code.
 *  To add one command the file had to be edited on several page.
 *
 *  To fight this and increase maintainability along reducing code complexity this
 *  "strategy" pattern has been implemented.
 *  Using this pattern the inverter has (except for the creation of the
 *  CSputnikCommand) no special knowledge about the commands it will issue.
 *
 *  The objects encapsulates all information needed to handle the command
 *   - information about the "command token" (token itself, lenght, max answer
 *     length)
 *   - information about the data we receive (type, capabilty name)
 *   - handles the registration of this data with the inverter.
 *
 *  This interface is the base class for the commands. There are two basic
 *  variantes of derived classes:
 *  - Most of the commands use simple datatype (integer, float, boolean ...)
 *  - Some commands needs special implementation (for example the Software-
 *    Version needs to assemble the information from two inverter queries)
 *
 *  For the first category, the templated "CSputnikCommand<type>" can be used,
 *  for the others derived classes like the "CSputnikCommandSoftwareVersion"
 *  come into the game.
 */

/*
 *
 * Created on: 19.05.2012
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
    /// Constructs the ISputnikCommand.
    /// If given, backoffstrategy will be then owned by this object and also
    /// freed on destruction.
    ISputnikCommand( const std::string &command, int max_answer_len,
            IInverterBase *inv, const std::string & capaname,
            ISputnikCommandBackoffStrategy *backoffstrategy = NULL );

    virtual ~ISputnikCommand();

    /** Returnes the maximium expected length (usually set at compile time) */
    virtual int GetMaxAnswerLen(void) {
        return max_answer_len;
    }

    /** Setter for the Max Answer Len, if needed*/
    virtual void SetMaxAnswerLen(int max) {
        max_answer_len = max;
    }

    /** Should we consider this command to be issued?
     * Note: If this function is overriden in your class, call
     * strat->ConsiderCommand -- if it returns false, you also return false.
     * \returns true if so, else do not issue command at this time. */
    virtual bool ConsiderCommand();

    /** Returns a const reference to the token string to be used for the communication
     * when requesting the data from the inverter.
     * (note: can be overriden for complex datas, for example if more than one command is
     * required to get thw whole set. */
    virtual const std::string& GetCommand(void);

    /** Helper: Return the length of the command to be issued. */
    virtual unsigned int GetCommandLen(void);

    /** Check if the token is handled by this instance.
     * \returns true if it is, else false
     *
     * For complex data, which is assembled from more than one command, this
     * needs be overridden.
    */
    virtual bool IsHandled(const std::string &token);

    /// handles the parsing, and handles the capability then.
    /// must be implemented in the derived class.
    /// NOTE: You must call strat->CommandAnswered in your derived class.
    virtual bool handle_token(const std::vector<std::string> &) = 0;

    /// command was sent, but no answer received
    /// (will only be called when there was no other error, like communication
    /// etc)
    virtual void CommandNotAnswered() {
        this->strat->CommandNotAnswered();
    }

    virtual void InverterDisconnected() {
        this->strat->Reset();
    }

protected:

    /** Makes the complete capability handling:
     *
     * Please see the overridden version (with capaname as parameter)
     * for details.
     */
    template <class T>
    void CapabilityHandling(T value) const throw() {
        this->CapabilityHandling<T>(value, this->capaname);
    };

    /** Makes the complete capability handling:
     * - Registers capability if not existing (incl. notifyng of the observers)
     * - Updates capability with new value (incl. notifying of observers.)
     * - Notifcation is only done when the value actually changes.
     * - Type-check of capabilities (throws if not) (only happens if capability
     *   handling is not done consequently, that is using types-defines in
     *   Capabilites.h)
     *
     *   \param value Value to be stored
     *   \param capname Capability-name.
     *
     *   \throw exception if types are mismatching.
     */
    template <class T>
    void CapabilityHandling(T value, const std::string &capname) const throw() {
        assert(inverter);
        CCapability *cap = inverter->GetConcreteCapability(capname);

        if (!cap) {
           IValue *v = new CValue<T>;
           ((CValue<T>*)v)->Set(value);
           cap = new CCapability(capname,v,inverter);
           inverter->AddCapability(cap);
           inverter->GetConcreteCapability(CAPA_CAPAS_UPDATED)->Notify();
           cap->Notify();
           return;
        }

        // Check for the type and throw an exception if not equal.
        // (as capabilites are created by this class, this should be not hapen)
         if ( CValue<T>::IsType(cap->getValue())) {
            CValue<T> *v = (CValue<T> *)cap->getValue();
            if (value != v->Get()) {
                v->Set(value);
                cap->Notify();
            }
            return;
        } else {
            std::bad_cast e;
            LOGERROR(inverter->logger,"Bad cast for command " + command);
            throw e;
        }
    }

protected:
    std::string command;
    int max_answer_len;
    IInverterBase *inverter;
    std::string capaname;
    ISputnikCommandBackoffStrategy *strat;
};

#endif /* ISPUTNIKCOMMAND_H_ */
