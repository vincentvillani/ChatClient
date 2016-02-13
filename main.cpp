/*
 * main.cpp
 *
 *  Created on: 13 Jan 2016
 *      Author: vincent
 */


#include <stdio.h>
#include <thread>

#include "ClientThreadFunctions.h"
#include "NetworkThreadFunctions.h"
#include "ClientData.h"
#include "NetworkData.h"
#include "MasterMailbox.h"


//TODO: Enter username on startup
//TODO: Auto connect to server without providing address
//TODO: Take a look at performance with optimisations on
//TODO: Test on OSX
//TODO: Add commands to see current connected users, exit the program, other useful stuff?




//This is the client thread
int main(int argc, const char* argv[])
{
	if(argc < 2)
	{
		printf("Usage: Username\n");
		exit(1);
	}


	const char* serverAddress = "Vincent-PC";

	ClientData* clientData = new ClientData(argv[1]);
	NetworkData* networkData = new NetworkData(serverAddress, argv[1]);


	MasterMailbox* masterMailbox = new MasterMailbox(clientData, networkData);


	//Start the networking thread
	std::thread networkingThread(NetworkThreadMain, networkData, masterMailbox);
	networkingThread.detach();

	ClientThreadMain(clientData, masterMailbox);


	delete clientData;
	delete networkData;
	delete masterMailbox;

	return 0;
}
