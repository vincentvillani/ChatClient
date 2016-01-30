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

class ChatData
{
public:

	ChatData();
	virtual ~ChatData();

	int maxRow;
	int maxCol;

	int currentBaseChatIndex; //Current row number on the screen, starts at 0
	std::vector<std::string> chatVector;

	std::string currentMessage;
};

#endif /* CHATDATA_H_ */
