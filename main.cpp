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
//TODO: Add commands to see current connected users, exit the program, other useful stuff?




//This is the client thread
int main(int argc, const char* argv[])
{
	if(argc < 2)
	{
		fprintf(stderr ,"Please provide a username when starting the program!\nEG: ChatClient YourUsername\n");
		exit(1);
	}


	const char* serverAddress = "62.210.115.135";

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
