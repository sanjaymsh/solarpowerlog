/* ----------------------------------------------------------------------------
 solarpowerlog
 Copyright (C) 2009  Tobias Frost

 This file is part of solarpowerlog.

 Solarpowerlog is free software; However, it is dual-licenced
 as described in the file "COPYING".

 For this file (ICommand.h), the license terms are:

 You can redistribute it and/or  modify it under the terms of the GNU Lesser
 General Public License (LGPL) as published by the Free Software Foundation;
 either version 3 of the License, or (at your option) any later version.

 This programm is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Library General Public
 License along with this proramm; if not, see
 <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------------
 */

/** \file ICommand.h
 *
 *  Created on: May 17, 2009
 *      Author: tobi
 */

#define DEPRECIATE_OLD_ICOMMAND

#ifndef ICOMMAND_H_
#define ICOMMAND_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <map>
#include <stdexcept>

#include <boost/any.hpp>

class ICommandTarget;

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
	ICommand(int command, ICommandTarget *target, std::map<std::string,
			boost::any> dat);

	ICommand(int command, ICommandTarget *target);

	virtual ~ICommand();

	void execute();

	int getCmd() const;

#ifdef DEPRECIATE_OLD_ICOMMAND
private:
#endif
	ICommand(int command, ICommandTarget *target, void *dat);
	void *getData() const;
	void setData(void *data) {
		this->data = data;
	}

#ifdef DEPRECIATE_OLD_ICOMMAND
public:
#endif


	/** Find Data in Command
	 *
	 * Returns the data associated in the map to the given key
	 *
	 * \in param key to search for
	 * \returns boost::any object with the data
	 *
	 * \throw std::invalid_argument { throws this if data is not existant. The
	 * data of the invalid_argument is the key which was not found }
	 */
	const boost::any findData(const std::string &key)
			throw (std::invalid_argument);

	/** Remove Data from Command
	 *
	 * Removes the named key from the data list of the command.
	 * \note: As the underlaying storage is a std::map,
	 * the associated boost::any object will be deleted.
	 */
	inline void RemoveData(const std::string &key) {
		dat.erase(key);
	}

	/** Add/Replace Data from the Command
	 *
	 * Add new data, or if the data is already existing, replace
	 * the data with the new one.
	 *
	*/
	void addData(const std::string &key, const boost::any &data) {
		if (dat.count(key)) {
			dat.erase(key);
		}
		dat.insert(std::pair<std::string, boost::any>(key, data));
	}

private:
	int cmd;
	ICommandTarget *trgt;
	void *data;

	std::map<std::string, boost::any> dat;

};

#endif /* ICOMMAND_H_ */
