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
                { 20002, NOT_FEEDING_OK, "Solar radiation too low" },
                { 20003, NOT_FEEDING_OK, "Inverter Starting up" },
                { 20004, FEEDING_MPP, "Feeding on MPP" },

                { 20006, FEEDING_MAXPOWER, "Feeding. Inverter at power limit" },
                { 20008, FEEDING, "Feeding" },

                { 20115, NOT_FEEDING_EXTEVENT, "Off-grid" },
                { 20116, NOT_FEEDING_EXTEVENT, "Grid Frequency too high" },
                { 20117, NOT_FEEDING_EXTEVENT, "Grid Frequency too low" },

                { 1,
                    STATUS_UNAVAILABLE,
                    "Unknown Statuscode -- PLEASE FILE A BUG WITH AS MUCH INFOS AS YOU CAN FIND OUT -- BEST, READ THE DISPLAY OF THE INVERTER." }, };

bool CSputnikCommandSYS::handle_token(const std::vector<std::string>& tokens) {

    if (tokens.size() != 3) return false;
    char *ptmp = NULL;

    int status = strtoul(tokens[1].c_str(), &ptmp, 16);
    if (!status || status == ULONG_MAX || ptmp == tokens[1].c_str()) return false;
    int status2 = strtoul(tokens[2].c_str(), &ptmp, 16);
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

    return true;
}
