/** \file ICommand.h
 *
 *  Created on: May 17, 2009
 *      Author: tobi
 */

#ifndef ICOMMAND_H_
#define ICOMMAND_H_

#include "patterns/ICommandTarget.h"

/** Encapsulates a command
 *
 * See the command pattern for details....
 *
 * Commands are used, for example, to schedule work,
 * to check for completion, etc...
 *
 * NOTE: The one that calls execute should delete the object afterwards!
 *
 */
class ICommand {
public:
	ICommand(int command, ICommandTarget *target, void *dat = 0);

	virtual ~ICommand();

	void execute();

    int getCmd() const;

    void *getData() const;

private:
	int cmd;
	ICommandTarget *trgt;
	void *data;


};

#endif /* ICOMMAND_H_ */
