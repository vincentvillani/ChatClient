/*
 * NetworkData.cpp
 *
 *  Created on: 23 Jan 2016
 *      Author: vincent
 */

#include "NetworkData.h"

NetworkData::NetworkData(std::string username)
{
	this->username = username;
	this->serverSocket = NULL;
	this->ioBuffer = new NetworkReadWriteBuffer(0);
	this->pollArray = new PollArray();
}

NetworkData::~NetworkData()
{
	if(serverSocket != NULL)
		delete serverSocket;

	if(ioBuffer != NULL)
		delete ioBuffer;

	if(pollArray != NULL)
		delete pollArray;
}

