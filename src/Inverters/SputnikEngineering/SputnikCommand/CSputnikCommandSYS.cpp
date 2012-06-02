/*
 * CSputnikCommandSYS.cpp
 *
 *  Created on: 23.05.2012
 *      Author: tobi
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "CSputnikCommandSYS.h"
#include "Inverters/Capabilites.h"
#include <string>

static const struct
{
    unsigned int code;
    enum InverterStatusCodes status;
    const char *description;
}
        statuscodes[] = {
            { 20001, FEEDING, "Operating" },
            { 20002, NOT_FEEDING_OK, "Solar radiation too low" },
            { 20003, NOT_FEEDING_OK, "Inverter starting up" },
            { 20004, FEEDING_MPP, "Feeding on MPP" },
            { 20005, FEEDING, "Feeding. Fan on" },
            { 20006, FEEDING_MAXPOWER, "Feeding. Inverter at power limit" },
            { 20008, FEEDING, "Feeding" },
            { 20009, FEEDING_MAXPOWER, "Feeding. Current limitation (DC)" },
            { 20010, FEEDING_MAXPOWER, "Feeding. Current limitation (AC)" },
            { 20011, FEEDING_WARNING, "Testmode" },
            { 20012, FEEDING_WARNING, "Remotely controlled" },
            { 20013, NOT_FEEDING_OK, "Restart delay" },

            { 20110, NOT_FEEDING_ERROR, "Internal voltage too high." },
            { 20111, NOT_FEEDING_ERROR, "Surge" },
            { 20112, NOT_FEEDING_ERROR, "Overload." },

            { 20114, NOT_FEEDING_ERROR, "Error current too high." },
            { 20115, NOT_FEEDING_EXTEVENT, "Off-grid" },
            { 20116, NOT_FEEDING_EXTEVENT, "Grid frequency too high" },
            { 20117, NOT_FEEDING_EXTEVENT, "Grid frequency too low" },
            { 20118, FEEDING_WARNING, "Islanding -- Operating in island mode." },
            { 20119, NOT_FEEDING_EXTEVENT, "Grid quality too low" },

            { 20122, NOT_FEEDING_EXTEVENT, "Grid voltage too high" },
            { 20123, NOT_FEEDING_EXTEVENT, "Grid voltage too low" },
            { 20124, NOT_FEEDING_EXTEVENT, "Overtemperature" },
            { 20125, STATUS_UNAVAILABLE, "Assymtetric Currents" },
            { 20126, STATUS_UNAVAILABLE, "Error external input 1" },
            { 20127, STATUS_UNAVAILABLE, "Error external input 2" },
            { 20129, STATUS_UNAVAILABLE, "Phase sequence wrong" },
            { 20130, STATUS_UNAVAILABLE, "Wrong Device" },
            { 20131, STATUS_UNAVAILABLE, "Main switch off" },
            { 20132, STATUS_UNAVAILABLE, "Diode overtemperature" },
            { 20133, STATUS_UNAVAILABLE, "Fan broken" },

            { 20166, NOT_FEEDING_EXTEVENT, "Grid frequency too high (20166)" },
            { 20167, NOT_FEEDING_EXTEVENT, "Grid frequency too low (20167)" },


            { 1,
                STATUS_UNAVAILABLE, "UNKNOWN -- Please file a bug "
                "with as much information as you have, and please read the display of your inverter." }, };

CSputnikCommandSYS::CSputnikCommandSYS( IInverterBase *inv,
        ISputnikCommandBackoffStrategy *backoff ) :
        ISputnikCommand("SYS", 10, inv, "", backoff), laststatuscode(0), secondparm_sys(
                0)
{
}

bool CSputnikCommandSYS::handle_token(const std::vector<std::string>& tokens) {

    if (tokens.size() != 3) return false;
    char *ptmp = NULL;

    unsigned long status = strtol(tokens[1].c_str(), &ptmp, 16);
    if (!status || status == ULONG_MAX || ptmp == tokens[1].c_str()) return false;
    unsigned long status2 = strtol(tokens[2].c_str(), &ptmp, 16);
    if (ptmp == tokens[1].c_str()) return false;

    if (status2 && status2 != secondparm_sys) {
        secondparm_sys = status2;
        LOGINFO(inverter->logger, "Received an unknown SYS response. Please file a bug"
                << " along with the following: " << tokens[0] << ","
                << tokens[1] << "," << tokens[2] << " and check the Inverter's display for more information.");
    }

    int i = 0;
    do {
        if (statuscodes[i].code == status)
            break;
    } while (statuscodes[++i].code != (unsigned int) -1);

    if (laststatuscode != (unsigned int) -1 && statuscodes[i].code
            == (unsigned int) -1) {
        LOGINFO(inverter->logger, "SYS reported an (too us) unknown status code of "
                << tokens[0] << "=" << tokens[1] << "," << tokens[2]
        );
        LOGINFO (inverter->logger,
                " PLEASE file along with all information you have, for example,"
                << " reading the display of the inverter and of the info given above."
        );
    }
    laststatuscode = statuscodes[i].code;
    CapabilityHandling<CAPA_INVERTER_STATUS_TYPE>(status,
        CAPA_INVERTER_STATUS_NAME);

    CapabilityHandling<CAPA_INVERTER_STATUS_READABLE_TYPE>(
        statuscodes[i].description, CAPA_INVERTER_STATUS_READABLE_NAME);

    this->strat->CommandAnswered();
    return true;
}
