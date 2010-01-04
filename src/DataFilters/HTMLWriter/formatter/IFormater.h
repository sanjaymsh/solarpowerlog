/*
 * IFormater.h
 *
 *  Created on: Jan 4, 2010
 *      Author: tobi
 */

#ifndef IFORMATER_H_
#define IFORMATER_H_

#include <string>

/** IFormater a strategies for reformatting capabilites for the CHTML Writer
 * output. They take a string, a configuration and return the reformatted
 * result.*/
class IFormater
{
public:
	static IFormater* Factory(const std::string &spec, const std::string &configpath);

	virtual bool Format(const std::string &what, std::string &store) = 0;

	virtual ~IFormater() {};

protected:
	IFormater(const std::string &configpath) { config = configpath; }

private:
	IFormater() {};

protected:
	std::string config;
};

#endif /* IFORMATER_H_ */
