/*
 * ChatClient.h
 *
 *  Created on: 13 Jan 2016
 *      Author: vincent
 */

#ifndef CHATCLIENT_H_
#define CHATCLIENT_H_

#include "ClientThreadPool.h"

#include "Socket.h"

class ChatClient
{
	ClientThreadPool _workPool;
	Socket* _serverSocket;

public:
	ChatClient();
	virtual ~ChatClient();


};

#endif /* CHATCLIENT_H_ */
