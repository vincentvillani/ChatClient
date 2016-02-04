/*
 * ChatData.cpp
 *
 *  Created on: 30 Jan 2016
 *      Author: vincent
 */

#include "ChatData.h"

ChatData::ChatData()
{
	baseViewableRow = 0;
	maxRow = 0;
	maxCol = 0;
	totalMessageRows = 0;
}

ChatData::~ChatData()
{
	for(uint32_t i = 0; i < messageVector.size(); ++i)
	{
		delete messageVector[i];
	}
}

