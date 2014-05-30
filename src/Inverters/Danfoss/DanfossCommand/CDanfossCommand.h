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
 * CDanfossCommand.h
 *
 *  Created on: 30.05.2014
 *      Author: tobi
 */

#ifndef CDANFOSSCOMMAND_H_
#define CDANFOSSCOMMAND_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_INV_DANFOSS

#include <string>
#include <vector>
#include <stdlib.h>

#include <boost/type_traits/is_convertible.hpp>

#include "Inverters/interfaces/InverterBase.h"
#include "Inverters/SputnikEngineering/SputnikCommand/ISputnikCommand.h"
#include "patterns/CValue.h"
#include "Inverters/Capabilites.h"
#include "configuration/ILogger.h"


std::string mhexdump(const std::string &s) {

    std::string st;
    char buf[32];
    for (unsigned int i = 0; i < s.size(); i++) {
        sprintf(buf, "%02x", (unsigned char)s[i]);
        st = st + buf;
        if (i && i % 16 == 0)
        st = st + "\n";
        else
        st = st + ' ';
    }
    return st;
}

/* Defines and handles a single commmand the Danfoss Inverter.
 *
 * CDanfossCommand (CDC) will in the long term replace the handling code in
 * CDanfossInverter for an object orientated approach.
 *
 * The object will store all knowledge about an individual command, allows the
 * parsing and eventually will also take over the Capabilities handling.
 *
 * It will also allow an strategy to wisely let the command itself decide when it
 * should be issued and how often, and to probe if an actual command is indeed supported
 * by the inverter.
 *
 * The design pattern is the same as the CSputnikCommand; (Indeed, it would probably make
 * sense to generalize the interface so that both implementations use the same base-class)
 * That's a TODO ... Thats true for the interface as well (which maybe only needs a
 * rename and minor tweaks on the interfaces...
 *
 * However, the Danfoss protocol ist quite straight forward:
 * Telegram format
 *|-------------------|============================================|-----------|
 *|  Start of frame   ||              Header          |   Data   || EOF       |
 *| 0x7e | Adr | Ctrl ||  Src  | Dest | Size  | Type  |  10Bytes || CRC | Stp |
 *|indx               ||   0      2      4       6         7-15  ||           |
 *|-------------------|============================================|---------- |
 *
 * The CDanfossCommand will receive the "framed" area in a
 * std::string. (It will get the complete Message part as there might be some
 * use-cases for the bits in the headers later; for example the ping
 * and node info queries...)
 * However, the Inverter-Class will pre-validate already certain fields,
 * especially the message header. (addressing, error signaling in the header)
 * and drop/retry the data/message if necessary.
 *
 * On outgoing messages, the CDanfossCommand is also expected to prepare the complete
 * frame. However, Source and Destination address will be filled (overwritten) by the
 * Inverter Class.
 *
*/

#define DANFOSS_POS_HDR_SOURCE (0)
#define DANFOSS_POS_HDR_DEST   (2)
#define DANFOSS_POS_HDR_SIZE   (4)
#define DANFOSS_POS_HDR_TYPE   (5)

#define DANFOSS_POS_DAT_DOCTYPE (6)
#define DANFOSS_POS_DAT_DEST    (7)
#define DANFOSS_POS_DAT_SRC     (8)
#define DANFOSS_POS_DAT_PARAM   (9)
#define DANFOSS_POS_DAT_SUBPARAM (10)
#define DANFOSS_POS_DAT_DTYPE   (11)
#define DANFOSS_POS_DAT_VALUE0   (12)
#define DANFOSS_POS_DAT_VALUE1   (13)
#define DANFOSS_POS_DAT_VALUE2   (14)
#define DANFOSS_POS_DAT_VALUE3   (15)
#define DANFOSS_BLOCK_SIZE   (16)

namespace DanfossCommand {
   enum typespec {
        type_boolean = 1,
        type_s8 = 2,
        type_s16 = 3,
        type_s32 = 4,
        type_u8 = 5,
        type_u16 = 6,
        type_u32 = 7,
        type_float = 8,
        // not imoplemented:
        type_notdefined = 0,
        type_visible_string = 9,
        type_packed_bytes = 10,
        type_packed_words = 11,
    };
}

/** Command Class template to aid decoding data from the Danfoss inverters.*/
template<class T>
class CDanfossCommand : public ISputnikCommand
{
public:
    /*** CDanfossCommand Constructor.
     *
     * The class needs a template arg which specifies the datatype for the Capabiltiy
     *
     * \param logger logger to be used for debugging
     * \param paramindex Index of the data this command handles (needed for the protocol)
     * \param subindex Index of the data this command handles (needed for the protocol)
     * \param moduleid Id of the target module (also needed for the protocol to talk to the inverter)
     * \param type the datatype the inverter speaks with that data
     *      (not necessarily same as the template type -- we will convert if possible.
     * \param inv pointer to the inverter owning this command
     * \param capaname the Capability to be registered under this name
     * \param backoff algortithm, if desired. default is "every time".
     *
     */
    CDanfossCommand(ILogger &logger,
        const uint8_t paramindex,
        const uint8_t subparamindex,
        const uint8_t moduleid,
        enum DanfossCommand::typespec type,
        IInverterBase *inv,
        const std::string & capname,
        ISputnikCommandBackoffStrategy *backoff = NULL)
        : ISputnikCommand(logger, "", 16, inv, capname, backoff),
            _paramindex(paramindex),
            _subparamindex(subparamindex),
            _moduleid(moduleid),
            _typeinfo(type)
    {
        // Lets initilize the command block...
        char tmp[DANFOSS_BLOCK_SIZE];
        tmp[DANFOSS_POS_HDR_SOURCE] = 0;
        tmp[DANFOSS_POS_HDR_SOURCE+1] = 0;
        tmp[DANFOSS_POS_HDR_DEST] = 0;
        tmp[DANFOSS_POS_HDR_DEST+1] = 0;
        tmp[DANFOSS_POS_HDR_SIZE] = 10;
        tmp[DANFOSS_POS_HDR_TYPE] = 1;

        tmp[DANFOSS_POS_DAT_DOCTYPE] = 0xC8;
        tmp[DANFOSS_POS_DAT_DEST] = _moduleid;
        tmp[DANFOSS_POS_DAT_SRC] = 0xD0;
        tmp[DANFOSS_POS_DAT_PARAM] = _paramindex;
        tmp[DANFOSS_POS_DAT_SUBPARAM] = _subparamindex;
        tmp[DANFOSS_POS_DAT_DTYPE] = 0x80 | _typeinfo;
        tmp[DANFOSS_POS_DAT_VALUE0] = 0;
        tmp[DANFOSS_POS_DAT_VALUE1] = 0;
        tmp[DANFOSS_POS_DAT_VALUE2] = 0;
        tmp[DANFOSS_POS_DAT_VALUE3] = 0;

        _commandblock.assign(tmp,DANFOSS_BLOCK_SIZE);
        LOGDEBUG(logger, "_commandblock for " << capname << " is " << mhexdump(_commandblock));
    }

    virtual bool handle_token(const std::string &token) {
        // Some common checks on the datablock:
        // Length constraint: Its always 6 bytes header and 10 bytes data.
        if (token.length() != DANFOSS_BLOCK_SIZE) {
            LOGDEBUG(logger, "CDanfossCommand: Data too short while handling "
                     << this->capaname);
            return false;
        }
        uint8_t type = token[DANFOSS_POS_DAT_DTYPE];
        // check if type higher bits indicates an error
        if (type & 0x20) {
            LOGINFO(logger,
                    "CDanfossCommand: Reply indicates unsupported data for "
                    << this->capaname);
            return false;
        }

        if ((type & 0xF0) != 0x40) {
            LOGINFO(logger,
                    "CDanfossCommand: Reply has invalid flags for "
                    << this->capaname);
            return false;
        }

        type &= 0x0F;
        try {
            T temp = handle_token_and_type(token, type);
            CapabilityHandling<T>(temp);

        } catch (...) {
            return false;
        }
        this->strat->CommandAnswered();
        return true;
    }

     /** Returns a const reference to the token string to be used for the communication
      * when requesting the data from the inverter.
      * (note: can be overriden for complex datas, for example if more than one command is
      * required to get thw whole set. */
     virtual const std::string& GetCommand(void) {
         return _commandblock;
     }

     /** Check if the token is handled by this instance.
      *
      * \param token reveived message.
      * \returns true if it is, else false
      *
      * Needs to be overridden for Danfoss inverters...
      *
     */
    virtual bool IsHandled(const std::string &token) {

        if (token[DANFOSS_POS_DAT_PARAM] == this->_paramindex &&
            token[DANFOSS_POS_DAT_SUBPARAM] == this->_subparamindex)
            {
            return true;
        }
        return false;
    }

private:
     /** Type and value handling. */
    T handle_token_and_type(const std::string &token,
        uint8_t type) const throw () {

        bool type_ok = false;
        T result;

        char valuearray[4];
        valuearray[0] = token[DANFOSS_POS_DAT_VALUE0];   // LSB!
        valuearray[1] = token[DANFOSS_POS_DAT_VALUE1];
        valuearray[2] = token[DANFOSS_POS_DAT_VALUE2];
        valuearray[3] = token[DANFOSS_POS_DAT_VALUE3];   // MSB!

        switch (type)
        {
            case DanfossCommand::type_boolean:
                // boolean
                if (boost::is_convertible<bool, T>::value) {
                    type_ok = true;
                    result = token[DANFOSS_POS_DAT_VALUE0];
                }
            break;

                // signed values
            case DanfossCommand::type_s8: // 8-bits
                // signed -- expand based on signedness
                if (valuearray[0] & 0x80) {
                    valuearray[1] = 0xff;
                }
                else {
                    valuearray[1] = 0x00;
                }
                // fall-though OK
            case DanfossCommand::type_s16: // 16-bits
                if (valuearray[1] & 0x80) {
                    valuearray[2] = 0xff;
                    valuearray[3] = 0xff;
                }
                else {
                    valuearray[2] = 0x00;
                    valuearray[3] = 0x00;
                } // fall-though OK
            case DanfossCommand::type_s32: // 32-bits
                if (boost::is_convertible<signed long, T>::value) {
                    type_ok = true;
                    signed long tmp = valuearray[0]
                                      + (valuearray[1] << 8)
                                      + (valuearray[2] << 16)
                                      + (valuearray[3] << 24);
                    result = tmp;
                }
            break;

                // unsigned values
            case DanfossCommand::type_u8: // 8-bit
                valuearray[1] = 0x00;
                // fall-though OK
            case DanfossCommand::type_u16: // 16-bit
                valuearray[2] = 0x00;
                valuearray[3] = 0x00;
                // fall-though OK
            case DanfossCommand::type_u32: // 32-bit
                if (boost::is_convertible<unsigned long, T>::value) {
                    type_ok = true;
                    unsigned long tmp = valuearray[0]
                                        + (valuearray[1] << 8)
                                        + (valuearray[2] << 16)
                                        + (valuearray[3] << 24);
                    result = tmp;
                }
            break;

                // float
            case DanfossCommand::type_float:
            {
                if (boost::is_convertible<float, T>::value) type_ok = true;
                unsigned long tmp = valuearray[0]
                                    + (valuearray[1] << 8)
                                    + (valuearray[2] << 16)
                                    + (valuearray[3] << 24);
                float *ptmp = (float*)&tmp;
                result = (*ptmp);
            }
            break;

            default:
                LOGDEBUG(
                    logger,
                    "Typehandling for type=" << type << " not implemetented. "
                    "Received while parsing for " << this->capaname);
            break;
        }

        if (!type_ok) {
            std::bad_cast e;
            LOGWARN(logger,
                    "Unexpected datatype while parsing for " << this->capaname);
            throw e;
        }

        return result;
    }

    /** DONT call this function, we have our own.
     *
     * TODO needs to be implemented until ISputnikCommand cleaned up to provide
     * an more general interface.
     *
     */
    virtual bool handle_token(const std::vector<std::string> &) {
        assert(0);
    }

private:

    uint8_t _paramindex;
    uint8_t _subparamindex;
    uint8_t _moduleid;
    uint8_t _typeinfo;

    std::string _commandblock;

};

#endif

#endif /* CDANFOSSCOMMAND_H_ */
