/** \file ICommandTarget.h
 *
 *  Created on: May 17, 2009
 *      Author: tobi
 */

#ifndef ICOMMANDTARGET_H_
#define ICOMMANDTARGET_H_

class ICommand;

/** Interface for a command's target.
 *
 *
 * TODO DOCUMENT ME!
 */
class ICommandTarget {
public:
	ICommandTarget();
	virtual ~ICommandTarget();

	virtual void ExecuteCommand(const ICommand *Command);
};

#endif /* COMMANDTARGET_H_ */
