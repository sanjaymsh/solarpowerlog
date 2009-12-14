/*
 * CAsyncCommand.h
 *
 *  Created on: Dec 14, 2009
 *      Author: tobi
 */

#ifndef CASYNCCOMMAND_H_
#define CASYNCCOMMAND_H_

#include <semaphore.h>
#include "patterns/ICommand.h"

class CAsyncCommand
{
public:
	enum Commando
	{
		DISCONNECT, /// Tear down a connection
		CONNECT, /// Connect
		SEND, /// Send data
		RECEIVE,
	/// Try to receive data
	};

	CAsyncCommand( enum Commando c, ICommand *callback, sem_t *sem = NULL )
	{
		this->c = c;
		if (!callback) {
			this->callback = new ICommand(NULL, NULL);
			private_icommand = true;
		} else {
			this->callback = callback;
			private_icommand = false;
		}

		this->sem = sem;
	}

	~CAsyncCommand()
	{
		if (private_icommand)
			delete callback;
	}

	void SetSemaphore( sem_t *sem )
	{
		this->sem = sem;
	}

	/** Handle this jobs completion by notifying the sender
	 *
	 */
	void HandleCompletion( void )
	{
		if (!private_icommand) {
			Registry::GetMainScheduler()->ScheduleWork(callback);
		} else {
			sem_post(sem);
		}
	}

	/** Is the asyncCommnd really async, or was it only pretended?
	 *
	 * As syncronous operations are also dispatched asynchronous,
	 * but we need a ICommand-object for this, we need the information
	 * if it is sync or not to decide when to delete the object.
	 * */
	bool IsAsynchronous()
	{
		return !private_icommand;
	}

	enum Commando c; ///< what to do

	/** callback for completion handling
	 * In this ICommand, the comand data is stored, results and data...
	 * This ICommand is privately created, if private_icommand is true,
	 * and will not be executed if so, but can be still be used for
	 * storage  */
	ICommand *callback;
private:
	sem_t *sem; ///< if not-null, use this semaphore to notify completion.
	/// \note: it is context specific to check if it worked out or not.
	/// The semaphore is intended to be used for the "synchronous fallback"

	bool private_icommand;

};
#endif /* CASYNCCOMMAND_H_ */
