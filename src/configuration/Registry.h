/* ----------------------------------------------------------------------------
   solarpowerlog
   Copyright (C) 2009  Tobias Frost

   This file is part of solarpowerlog.

   Solarpowerlog is free software; However, it is dual-licenced
   as described in the file "COPYING".

   For this file (Registry), the license terms are:

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

/*
 * Registry.h
 *
 *  Created on: May 9, 2009
 *      Author: tobi
 *
 * The Registry Class ist the central point to store configuration relevant for operations.
 *
 * Each Class requiring a configuration is free to use one of its objects.
 *
 */

#ifndef REGISTRY_H_
#define REGISTRY_H_


#include <utility>
#include <string>
#include <libconfig.h++>

class CWorkScheduler;

using namespace std;


class Registry {
public:

	static Registry& Instance() {
		// TODO Make sure this is thread safe (might already be, depending on
		// the instanciation time: When does C++ instanciate static Objects?)
		static Registry Instance;
		return Instance;
	}

	// C O N F I G U R A T I O N

	/* Shortcut to get the configuration.
	 * Please note, that it must be loaded beforehand. */
	static libconfig::Config* Configuration()
	{
		return Registry::Instance().Config;
	}

	bool LoadConfig(std::string name);

	libconfig::Setting & GetSettingsForObject(std::string section, std::string objname= "");



	// S C H E D U L E R

    void setMainScheduler(CWorkScheduler *mainscheduler)
    {
    	Registry::Instance().mainscheduler = mainscheduler;
    }

	static CWorkScheduler *GetMainScheduler(void) {
		return Registry::Instance().mainscheduler;
	}

private:
    CWorkScheduler *mainscheduler;

protected:
	Registry();
	Registry (const Registry& other) {};
	virtual ~Registry();

private:
	libconfig::Config *Config;



	bool loaded;


};

#endif /* REGISTRY_H_ */
