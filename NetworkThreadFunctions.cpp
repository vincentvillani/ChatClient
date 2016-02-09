/*
 * NetworkThreadFunctions.cpp
 *
 *  Created on: 23 Jan 2016
 *      Author: vincent
 */


#include "NetworkThreadFunctions.h"

#include "Network.h"
#include "NetworkCommandType.h"
#include "MasterMailbox.h"

#include <string.h>
#include <stdint.h>

#define SERVER_PORT_STRING "3490"


static void ConnectToServer(NetworkData* networkData, MasterMailbox* mailbox, const char* ip);

static void PollSocketsAndDoIO(NetworkData* networkData, MasterMailbox* masterMailbox);
static void PerformPendingWrites(NetworkData* networkData, MasterMailbox* masterMailbox);

static void TellClientMessageSent(uint16_t messageType, MasterMailbox* mailbox);



static void ConnectToServer(NetworkData* networkData, MasterMailbox* mailbox, const char* ip)
{
	networkData->serverSocket = NULL;

	//Setup a connection to the server
	//Get the address of the server
	int serverSocketHandle = -1;
	ADDRINFO hints;
	ADDRINFO* addressResults;
	int returnValue;

	//Clear the hint struct for use
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	returnValue = NetworkGetAddressInfo(ip, SERVER_PORT_STRING, &hints, &addressResults);

	if(returnValue < 0)
	{
		fprintf(stderr, "ChatClient::ChatClient: Error - %s\n", gai_strerror(returnValue));
		exit(1);
	}


	ADDRINFO* currentAddressInfo = addressResults;
	for(; currentAddressInfo != NULL; currentAddressInfo = currentAddressInfo->ai_next)
	{
		//printf("Hi!\n");

		serverSocketHandle = NetworkSocketCreate(currentAddressInfo->ai_family, currentAddressInfo->ai_socktype);

		//Unsucessfully got a socket descriptor
		if(serverSocketHandle == -1)
		{
			printf("Unsuccessfully got socket descriptor\n");
			continue;
		}

		//Set the socket to be useable, if the program crashes for some reason, this will remove this error message
		NetworkSocketSetReusable(serverSocketHandle, true);

		//Try to connect to the socket
		//bind will automatically be called using whatever local port the system decides
		returnValue = NetworkSocketConnect(serverSocketHandle, currentAddressInfo->ai_addr, currentAddressInfo->ai_addrlen);

		//Unsuccessfully connected
		if(returnValue == -1)
		{
			printf("Unsuccessfully connected to socket\n");
			continue;
		}

		//If we get here we have successfully connected, we can get out of the loop
		break;
	}

	//We have unsuccessfully connected to the server
	if(currentAddressInfo == NULL)
	{
		fprintf(stderr, "Unable to connect to the server\n");
		exit(1);
	}

	freeaddrinfo(addressResults);

	networkData->serverSocket = new Socket(serverSocketHandle, currentAddressInfo->ai_addr);

	//Set the socket to be non-blocking
	NetworkSocketSetNonBlock(networkData->serverSocket->handle);

	//Set the socket handle on the io buffer
	networkData->ioBuffer->socketHandle = networkData->serverSocket->handle;


	//Send the username along to the server
	NetworkThreadSendUsername(networkData, mailbox, networkData->username);

	//printf("Successfully connected to server!\n");
}



void PollSocketsAndDoIO(NetworkData* networkData, MasterMailbox* masterMailbox)
{



	//Get the total number of client sockets and see if there is any data waiting on them
	POLLFD* pollStructs = networkData->pollArray->getArrayWithSize(1);

	pollStructs[0].fd = networkData->serverSocket->handle;

	if(networkData->ioBuffer->writeBufferQueue.size())
		pollStructs[0].events = POLLIN | POLLOUT;
	else
		pollStructs[0].events = POLLIN;


	//Do the poll
	int numberOfSocketsReadyForRead = NetworkSocketPoll(pollStructs, 1, 50);

	//An error has occured
	if(numberOfSocketsReadyForRead == -1)
	{
		fprintf(stderr, "ChatServer::PollClientSocketsForWrite: %s", strerror(errno));
		return;
	}


	if(numberOfSocketsReadyForRead == 0)
		return;


	//There is something to do

	//Is there something to read?
	if(pollStructs[0].revents & POLLIN)
	{
		//Do a read
		//TODO: IMPLEMENT THIS
	}

	//Is the socket ready for a write?
	if(pollStructs[0].revents & POLLOUT)
	{
		//Try and do all the writes we can
		PerformPendingWrites(networkData, masterMailbox);
	}

}

void PerformPendingWrites(NetworkData* networkData, MasterMailbox* masterMailbox)
{
	//Try to write as much as possible to the write buffer
	while(networkData->ioBuffer->writeBufferQueue.size())
	{
		NetworkWriteBuffer* currentWriteBuffer = networkData->ioBuffer->writeBufferQueue.front();
		char* currentBuffer = currentWriteBuffer->data;

		int bytesJustWritten = NetworkSocketSend(networkData->serverSocket->handle, currentBuffer + currentWriteBuffer->sentBytes,
				currentWriteBuffer->remainingBytesToSend(), 0);

		//We can't write any more, lets just leave
		if(bytesJustWritten == -1)
		{
			break;
		}

		//Update the amount of bytes sent
		currentWriteBuffer->sentBytes += bytesJustWritten;

		//Have we sent this entire message?
		if(currentWriteBuffer->remainingBytesToSend() == 0)
		{
			//We have sent this whole message
			delete currentWriteBuffer;

			//Remove it from the queue
			networkData->ioBuffer->writeBufferQueue.pop();
		}
	}
}




void NetworkThreadMain(NetworkData* networkData, MasterMailbox* mailbox, const char* ip)
{
	ConnectToServer(networkData, mailbox, ip);

	while(true)
	{

		std::unique_lock<std::mutex> workQueueLock(networkData->mutex);

		//Is there any work to do?
		bool workToDo = networkData->conditionVariable.wait_for(workQueueLock, std::chrono::milliseconds(1), [&]{return networkData->workQueue.size();} );

		//Do all the work
		if(workToDo)
		{
			while(networkData->workQueue.size())
			{
				std::function<void()> workItem = networkData->workQueue.front();
				networkData->workQueue.pop();
				workItem(); //Do the work

			}

			//Release the mutex
			workQueueLock.unlock();
		}

		//Do read and write IO
		PollSocketsAndDoIO(networkData, mailbox);
	}
}



void NetworkThreadSendUsername(NetworkData* networkData, MasterMailbox* mailbox, std::string newUsername)
{
	//Format the new data to send
	uint8_t usernameLength = newUsername.size() + 1;
	uint16_t messageType = NETWORK_USERNAME;
	uint32_t totalMessageLength = sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint8_t) + usernameLength;



	//printf("Name: %s\n", newUsername.c_str());


	char* data = (char*)malloc(totalMessageLength);
	char terminator = '\0';

	memcpy(data, &totalMessageLength, sizeof(uint32_t)); //Total message length
	memcpy(data + 4, &messageType, sizeof(uint16_t)); //message type
	memcpy(data + 6, &usernameLength, sizeof(uint8_t)); //username length
	memcpy(data + 7, newUsername.c_str(), newUsername.size());
	memcpy(data + 7 + newUsername.size(), &terminator, 1);

	//printf("Char name: %s\n", (data + 7));

	//Create a write buffer and add it to the iobuffer queue
	NetworkWriteBuffer* networkWriteBuffer = new NetworkWriteBuffer(totalMessageLength, data, networkData->ioBuffer->socketHandle, messageType);
	networkData->ioBuffer->writeBufferQueue.push(networkWriteBuffer); //Don't need a mutex, because only this thread should be using this buffer

	//Tell the server that we've sent a message
	TellClientMessageSent(messageType, mailbox);

}



void NetworkThreadSendChatMessage(NetworkData* networkData, std::string currentMessage)
{
	uint32_t chatMessageLength = currentMessage.size() + 1; //Plus one for null terminator
	uint32_t totalMessageByteSize = sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint32_t) + chatMessageLength;
	uint16_t messageType = NETWORK_CHAT_MESSAGE;

	char* data = (char*)malloc(totalMessageByteSize);
	char terminator = '\0';

	memcpy(data, &totalMessageByteSize, sizeof(uint32_t)); //Total message length
	memcpy(data + 4, &messageType, sizeof(uint16_t)); //message type
	memcpy(data + 6, &chatMessageLength, sizeof(uint32_t)); //chat message length
	memcpy(data + 10, currentMessage.c_str(), currentMessage.size());
	memcpy(data + 10 + currentMessage.size(), &terminator, 1);

	//Create a write buffer and add it to the iobuffer queue
	NetworkWriteBuffer* networkWriteBuffer = new NetworkWriteBuffer(totalMessageByteSize, data, networkData->ioBuffer->socketHandle, messageType);
	networkData->ioBuffer->writeBufferQueue.push(networkWriteBuffer); //Don't need a mutex, because only the network thread should be using this buffer
}


void TellClientMessageSent(uint16_t messageType, MasterMailbox* mailbox)
{
	switch(messageType)
	{
		case NETWORK_USERNAME:
			mailbox->NetworkTellServerThreadUsernameUpdated();
			break;
		default:
			fprintf(stderr, "Message type doesn't match any known type\n");
			break;
	}
}

