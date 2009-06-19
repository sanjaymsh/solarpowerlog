#ifndef _BASIC_COMMANDS_H
#define _BASIC_COMMANDS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

enum BasicCommands {

	/// reserverd for transport algorithms which can send events
	/// on receiption (planned... therefore TODO )
	CMD_RECEIVED ,
	// For now, we reserve 1000 cmds for our purpose.
	// but please use this define for your commands.
	CMD_USER = 1000
};


#endif
