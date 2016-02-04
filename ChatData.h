/*
 * ChatData.h
 *
 *  Created on: 30 Jan 2016
 *      Author: vincent
 */

#ifndef CHATDATA_H_
#define CHATDATA_H_

#include <vector>
#include <string>
#include <stdint.h>

#include "UIMessage.h"

class ChatData
{
public:

	ChatData();
	virtual ~ChatData();

	int maxRow;
	int maxCol;

	uint32_t totalMessageRows;
	uint32_t baseViewableRow; //How many rows away from the last row of the most recent message is the row at the very bottom of the chat section? (ROWS, NOT MESSAGES)
	std::vector<UIMessage*> messageVector;
	std::string currentMessage;
};

#endif /* CHATDATA_H_ */
