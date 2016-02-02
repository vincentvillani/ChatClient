/*
 * UIMessage.cpp
 *
 *  Created on: 2 Feb 2016
 *      Author: vincent
 */

#include "UIMessage.h"

#include <stdlib.h>

UIMessage::UIMessage(std::string username, std::string message)
{
	this->username = username;
	this->message = message;
	this->UIRowsUsed = 0;

}

UIMessage::~UIMessage()
{

}

