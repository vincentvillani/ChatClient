/*
 * UIMessage.h
 *
 *  Created on: 2 Feb 2016
 *      Author: vincent
 */

#ifndef UIMESSAGE_H_
#define UIMESSAGE_H_

#include <stdint.h>
#include <string>

class UIMessage
{

public:
	UIMessage(std::string username, std::string message);
	virtual ~UIMessage();

	std::string message;
	std::string username;
	uint32_t UIRowsUsed;
};

#endif /* UIMESSAGE_H_ */
