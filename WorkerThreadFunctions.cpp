/*
 * WorkerThreadFunctions.cpp
 *
 *  Created on: 13 Jan 2016
 *      Author: vincent
 */
#include "WorkerThreadFunctions.h"

#include<stdio.h>

#include "Network.h"
#include "Socket.h"

void SendMessageToServer(Socket* serverSocket, char* message, uint32_t messageLength)
{
	//Lock the write mutex
	std::lock_guard<std::mutex> writeLockGuard(serverSocket->writeMutex);

	int returnValue;

	returnValue = NetworkSocketSend(serverSocket->handle, (void*)message, messageLength, 0);

	//TODO: Handle this error
	if(returnValue == -1)
	{
		printf("Something has gone wrong...\n");
	}

	//printf("Message sent...\n");

}

