/*
 * IFormater.h
 *
 *  Created on: Jan 4, 2010
 *      Author: tobi
 */

#ifndef IFORMATER_H_
#define IFORMATER_H_

#include <string>
#include <vector>

/** IFormater a strategies for reformatting capabilites for the CHTML Writer
 * output. They take a string, a configuration and return the reformatted
 * result.
 *
 * It also contains its factory as a static member function.
 * */
class IFormater
{
public:
	static IFormater* Factory(const std::string &spec);

	/** This is the workhorse of the stragegy pattern:
	 * The format takes a refrence string of the content to be modified, a
	 * reference where it should suppose to write the result (note: might be
	 * same refrence.)
	 * and a vector of the formatters' configuration.
	 * The vector contains:
	 * [0] - formatter name (needed by caller, ignore)
	 * [1] - where to stored (needed by caller, ignore)
	 * [2...] - parameters supplied by user
	 */
	virtual bool Format(const std::string &what, std::string &store,
			const std::vector<std::string> &parameters) = 0;

	virtual ~IFormater()
	{
	}

protected:
	IFormater()
	{
	}

};

#endif /* IFORMATER_H_ */
