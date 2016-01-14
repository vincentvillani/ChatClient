/*
 * ChatClient.cpp
 *
 *  Created on: 13 Jan 2016
 *      Author: vincent
 */

#include "ChatClient.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <functional>
#include <stdlib.h>
#include <thread>
#include <chrono>

#include "Network.h"
#include "WorkerThreadFunctions.h"
#include "Socket.h"

#define SERVER_PORT_STRING "3490"

ChatClient::ChatClient()
{
	_serverSocket = NULL;

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

	returnValue = NetworkGetAddressInfo(NULL, SERVER_PORT_STRING, &hints, &addressResults);

	if(returnValue < 0)
	{
		fprintf(stderr, "ChatClient::ChatClient: Error - %s\n", gai_strerror(returnValue));
		exit(1);
	}


	ADDRINFO* currentAddressInfo = addressResults;
	for(; currentAddressInfo != NULL; currentAddressInfo = currentAddressInfo->ai_next)
	{
		printf("Hi!\n");

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
		returnValue = NetworkSocketConnect(serverSocketHandle, currentAddressInfo);

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

	//Make a copy of the socket address before setting it
	uint32_t copiedSocketAddressSize = sizeof(currentAddressInfo->ai_addr);
	SOCKADDR* copiedSocketAddress = (SOCKADDR*)malloc(sizeof(copiedSocketAddressSize));
	memcpy(copiedSocketAddress, currentAddressInfo->ai_addr, copiedSocketAddressSize);

	_serverSocket = new Socket(serverSocketHandle, copiedSocketAddress);

	freeaddrinfo(addressResults);




	//printf("Successfully connected to server!\n");

	//TODO: REMOVE THIS DEBUG STUFF
	char* username = "Vincent";
	char* message = "Wow this worked!?";
	uint32_t usernameLength = strlen(username) + 1;
	uint32_t messageLength = strlen(message) + 1;

	//This length does not include this uint32_t
	uint32_t totalMessageLength = sizeof(uint32_t) * 2 + usernameLength + messageLength;

	//Allocate enough space to store the whole message
	char* completeMessage = (char*)malloc(totalMessageLength + sizeof(uint32_t));

	//Write the data into the completeMessageBuffer
	memcpy(completeMessage, &totalMessageLength, sizeof(uint32_t));

	memcpy(completeMessage + sizeof(uint32_t), &usernameLength, sizeof(uint32_t));
	memcpy(completeMessage + sizeof(uint32_t) * 2, username, usernameLength);

	memcpy(completeMessage + (sizeof(uint32_t) * 2) + usernameLength, &messageLength, sizeof(uint32_t));
	memcpy(completeMessage + (sizeof(uint32_t) * 3) + usernameLength, message, messageLength);

	printf("TotalLength: %u\n", (uint32_t) (*completeMessage));
	printf("usernameLength: %u\n", (uint32_t) *(completeMessage + sizeof(uint32_t)));

	printf("message: %s\n", completeMessage);

	_workPool.addToWorkBuffer(std::bind(SendMessageToServer, _serverSocket, completeMessage, totalMessageLength + sizeof(uint32_t)));
	_workPool.debugTest();


	_workPool.addToWorkBuffer(std::bind(SendMessageToServer, _serverSocket, completeMessage, totalMessageLength + sizeof(uint32_t)));
	_workPool.debugTest();

	_workPool.addToWorkBuffer(std::bind(SendMessageToServer, _serverSocket, completeMessage, totalMessageLength + sizeof(uint32_t)));
	_workPool.debugTest();

	std::this_thread::sleep_for(std::chrono::seconds(5));

}

ChatClient::~ChatClient()
{
	if(_serverSocket != NULL)
		delete _serverSocket;
}

