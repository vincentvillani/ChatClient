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
#include <chrono>
#include <thread>

#define SERVER_PORT_STRING "3490"


static void ConnectToServer(NetworkData* networkData, MasterMailbox* mailbox);

static void PollSocketsAndDoIO(NetworkData* networkData, MasterMailbox* masterMailbox);


static void PerformPendingWrites(NetworkData* networkData, MasterMailbox* masterMailbox);
static void PerformPendingReads(NetworkData* networkData, MasterMailbox* masterMailbox);


static void TryProcessReadBuffer(NetworkData* networkData, MasterMailbox* masterMailbox, NetworkReadBuffer* readBuffer, int socketHandle);
static void ReadBufferToNetworkCommand(NetworkData* networkData, MasterMailbox* masterMailbox, NetworkReadBuffer* readBuffer, int socketHandle);


//static void ProcessUsernameChangedNetworkCommand(NetworkData* networkData, MasterMailbox* masterMailbox, NetworkReadBuffer* readBuffer, int socketHandle);
static void ProcessChatMessageNetworkCommand(NetworkData* networkData, MasterMailbox* masterMailbox, NetworkReadBuffer* readBuffer, int socketHandle);

static void ShiftReadBufferData(NetworkReadBuffer* readBuffer);

static void TellClientMessageSent(uint16_t messageType, MasterMailbox* mailbox);



static void ConnectToServer(NetworkData* networkData, MasterMailbox* mailbox)
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

	returnValue = NetworkGetAddressInfo(networkData->serverAddress, SERVER_PORT_STRING, &hints, &addressResults);

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
			//printf("Unsuccessfully connected to socket\n");
			continue;
		}

		//If we get here we have successfully connected, we can get out of the loop
		break;
	}

	freeaddrinfo(addressResults);

	//We have unsuccessfully connected to the server
	if(currentAddressInfo == NULL)
	{
		networkData->networkThreadShouldContinue = false;

		//fprintf(stderr, "Unable to connect to the server\n");
		return;
		//exit(1);
	}



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
		PerformPendingReads(networkData, masterMailbox);
	}

	//Is the socket ready for a write?
	if(pollStructs[0].revents & POLLOUT)
	{
		//Try and do all the writes we can
		PerformPendingWrites(networkData, masterMailbox);
	}

}

void PerformPendingReads(NetworkData* networkData, MasterMailbox* masterMailbox)
{
	int returnValue;

	//bool connectionWillClose = false;

	NetworkReadBuffer* readBuffer = networkData->ioBuffer->readBuffer;

	//Read as much data as we can
	while(true)
	{
		//Make sure there is enough space to store new data
		if(readBuffer->remainingStorageCapacity() == 0)
			readBuffer->grow();

		//Read the socket
		returnValue = NetworkSocketReceive(networkData->serverSocket->handle, readBuffer->data + readBuffer->bytesRead,
				readBuffer->remainingStorageCapacity(), 0);

		//Can't read anymore
		if(returnValue == -1)
		{
			break;
		}
		//Connection was closed by the server
		else if(returnValue == 0)
		{
			//Close the socket
			NetworkSocketClose(networkData->serverSocket->handle);

			//Alert the user
			masterMailbox->NetworkThreadDisconnectOccured();

			//Wait for a bit, then try to reconnect
			std::this_thread::sleep_for(std::chrono::seconds(3));
			ConnectToServer(networkData, masterMailbox);

			//We were unable to connect
			if(networkData->networkThreadShouldContinue == false)
			{
				//Send a message to the UI thread and tell it to shut down
				masterMailbox->NetworkThreadUnableToReconnect();

				return;
			}

			break;
		}
		//We must have read some data
		else
		{
			readBuffer->bytesRead += returnValue;
		}
	}

	//See if there is a whole message ready, process and send it off to the server
	TryProcessReadBuffer(networkData, masterMailbox, readBuffer, networkData->serverSocket->handle);

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


void TryProcessReadBuffer(NetworkData* networkData, MasterMailbox* masterMailbox, NetworkReadBuffer* readBuffer, int socketHandle)
{
	while(true)
	{
		//Check each time through the loop if there is enough data to process the next message
		if(readBuffer->currentMessageSize == 0 && readBuffer->bytesRead < 4)
			break;

		//Are we not currently waiting for a whole message and have enough data to know the length of the next message?
		if(readBuffer->currentMessageSize == 0 && readBuffer->bytesRead >= 4)
		{
			readBuffer->currentMessageSize = *((uint32_t*)readBuffer->data);
		}


		//At this point we should know the length of the next message

		//Do we have enough to process it?
		if(readBuffer->currentMessageSize <= readBuffer->bytesRead)
		{
			//Process it
			ReadBufferToNetworkCommand(networkData, masterMailbox, readBuffer, socketHandle);
		}
		else //Can't do anymore, break
			break;
	}
}


void ReadBufferToNetworkCommand(NetworkData* networkData, MasterMailbox* masterMailbox, NetworkReadBuffer* readBuffer, int socketHandle)
{

	uint16_t messageType = *(uint16_t*)(readBuffer->data + 4);

	//What is the message type
	switch(messageType)
	{
		case NETWORK_USERNAME:
			//No need for this for now, usernames are sent along with the messages
			//ProcessUsernameChangedNetworkCommand(networkData, masterMailbox, readBuffer, socketHandle);
			break;

		case NETWORK_CHAT_MESSAGE:
			ProcessChatMessageNetworkCommand(networkData, masterMailbox, readBuffer, socketHandle);
			break;

		case NETWORK_UNSET:
			break;

		default:
			fprintf(stderr, "ReadBufferToNetworkCommand: Received a network message type that is unset\n");
			break;

	}


}


/*
void ProcessUsernameChangedNetworkCommand(NetworkData* networkData, MasterMailbox* masterMailbox, NetworkReadBuffer* readBuffer, int socketHandle)
{
	//uint8_t usernameLength = (uint8_t)(readBuffer->data + 6);

	//4 + 2 + 1 byte, ONLY WORKS IF C STRING IS NULL TERMINATED!
	std::string* username = new std::string(readBuffer->data + 7);

	//printf("Username: %s", username->c_str());

	//Send the data over to the server thread
	masterMailbox->NetworkThreadUserLoginToServerThread(username, socketHandle);


	//Shift the readBuffer data across
	ShiftReadBufferData(readBuffer);
}
*/

void ProcessChatMessageNetworkCommand(NetworkData* networkData, MasterMailbox* masterMailbox, NetworkReadBuffer* readBuffer, int socketHandle)
{
	uint16_t usernameByteOffsetLength = *(uint16_t*)(readBuffer->data + 6);

	//Assumes the username is null terminated
	std::string username(readBuffer->data + 8);

	//4 + 2 + 2 + usernameDataLength + 4
	uint32_t chatMessageByteOffset = 12 + usernameByteOffsetLength; //4 + 2 + 4

	//Assumes the C String is NULL terminated
	std::string chatMessage(readBuffer->data + chatMessageByteOffset);

	//Send the data over to the client thread
	masterMailbox->NetworkThreadChatMessageReceived(username, chatMessage);

	//Shift the readBuffer across
	ShiftReadBufferData(readBuffer);
}



void ShiftReadBufferData(NetworkReadBuffer* readBuffer)
{
	//printf("%d", *(uint16_t*)(readBuffer->data + 19));
	//Move the data across
	memmove(readBuffer->data, readBuffer->data + readBuffer->currentMessageSize, readBuffer->bytesRead - readBuffer->currentMessageSize);

	//Update the buffer fields
	readBuffer->bytesRead -= readBuffer->currentMessageSize;
	readBuffer->currentMessageSize = 0;
}


void NetworkThreadMain(NetworkData* networkData, MasterMailbox* mailbox)
{
	//Setup
	ConnectToServer(networkData, mailbox);

	//We failed to connect
	if(networkData->networkThreadShouldContinue == false)
	{
		mailbox->NetworkThreadFailedToConnectToServer();
	}


	while(networkData->networkThreadShouldContinue)
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
	networkData->username = newUsername;

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
	uint16_t usernameLength = networkData->username.size() + 1; //Plus one for null terminator
	uint32_t chatMessageLength = currentMessage.size() + 1; //Plus one for null terminator
	uint16_t messageType = NETWORK_CHAT_MESSAGE;
									//Total message length, message type, username length, username data length, chat message length, chat message data length
	uint32_t totalMessageByteSize = sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t) + usernameLength + sizeof(uint32_t) + chatMessageLength;



	char* data = (char*)malloc(totalMessageByteSize);
	char terminator = '\0';

	uint32_t runningOffset = 0;

	memcpy(data, &totalMessageByteSize, sizeof(uint32_t)); //Total message length
	runningOffset += sizeof(uint32_t);

	memcpy(data + runningOffset, &messageType, sizeof(uint16_t)); //message type
	runningOffset += sizeof(uint16_t);

	memcpy(data + runningOffset, &usernameLength, sizeof(uint16_t));
	runningOffset += sizeof(uint16_t);

	memcpy(data + runningOffset, networkData->username.c_str(), networkData->username.size());
	runningOffset += networkData->username.size();

	memcpy(data + runningOffset, &terminator, 1);
	runningOffset += 1;


	memcpy(data + runningOffset, &chatMessageLength, sizeof(uint32_t)); //chat message length
	runningOffset += sizeof(uint32_t);

	memcpy(data + runningOffset, currentMessage.c_str(), currentMessage.size());
	runningOffset += currentMessage.size();

	memcpy(data + runningOffset, &terminator, 1);
	runningOffset += 1;

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

