/*
 * ChatClient.h
 *
 *  Created on: 13 Jan 2016
 *      Author: vincent
 */

#ifndef CLIENTDATA_H_
#define CLIENTDATA_H_

#include "WorkQueue.h"

#include <string>

#include "ChatData.h"

class ClientData : public WorkQueue
{

public:

	std::string username;

	ClientData(std::string username);
	virtual ~ClientData();

	ChatData chatData;
};

#endif /* CLIENTDATA_H_ */
