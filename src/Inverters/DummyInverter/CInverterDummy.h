/*
 * CInverterDummy.h
 *
 *  Created on: 17.07.2011
 *      Author: tobi
 */

#ifndef CINVERTERDUMMY_H_
#define CINVERTERDUMMY_H_

#ifdef HAVE_INV_DUMMY

#include "Inverters/interfaces/InverterBase.h"

class CInverterDummy: public IInverterBase
{
public:
	CInverterDummy(const string &name, const string & configurationpath);

	virtual ~CInverterDummy();

	virtual bool CheckConfig() {
		return this->connection->CheckConfig();
	}

	virtual void ExecuteCommand(const ICommand *Command);

private:
	enum CMDs {
		CMD_INIT
	};

};

#endif /* HAVE_INV_DUMMY */

#endif /* CINVERTERDUMMY_H_ */
