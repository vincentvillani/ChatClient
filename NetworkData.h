/*
 * NetworkData.h
 *
 *  Created on: 23 Jan 2016
 *      Author: vincent
 */

#ifndef NETWORKDATA_H_
#define NETWORKDATA_H_

#include "WorkQueue.h"

#include <string>

#include "Socket.h"
#include "NetworkReadWriteBuffer.h"
#include "PollArray.h"

class NetworkData : public WorkQueue
{

public:

	const char* serverAddress;

	std::string username;
	Socket* serverSocket;
	NetworkReadWriteBuffer* ioBuffer;
	PollArray* pollArray;

	bool networkThreadShouldContinue;

	NetworkData(const char* serverAddress, std::string username);
	virtual ~NetworkData();
};

#endif /* NETWORKDATA_H_ */
