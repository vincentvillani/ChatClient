/*
 * WorkerThreadFunctions.cpp
 *
 *  Created on: 13 Jan 2016
 *      Author: vincent
 */

#include "Network.h"


void sendMessageToServer(Socket* serverSocket, char* message, uint32_t messageLength)
{
	//Lock the write mutex
	std::lock_guard<std::mutex> writeLockGuard(serverSocket->writeMutex);

	int returnValue;

	returnValue = NetworkSocketSend(serverSocket->handle, (void*)message,messageLength, NULL);

}

