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

#include "Network.h"

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

	freeaddrinfo(addressResults);

	_serverSocket = new Socket(serverSocketHandle, currentAddressInfo->ai_addr);


	printf("Successfully connected to server!\n");

}

ChatClient::~ChatClient()
{
	if(_serverSocket != NULL)
		delete _serverSocket;
}

