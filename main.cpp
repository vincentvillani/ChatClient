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

//TODO: Handle disconnects and try reconnecting if possible
//TODO: Test on OSX

//This is the client thread
int main(int argc, const char* argv[])
{
	if(argc < 3)
	{
		printf("Usage: IPAddress Username\n");
		exit(1);
	}

	ClientData* clientData = new ClientData(argv[2]);
	NetworkData* networkData = new NetworkData(argv[2]);


	MasterMailbox* masterMailbox = new MasterMailbox(clientData, networkData);


	//Start the networking thread
	std::thread networkingThread(NetworkThreadMain, networkData, masterMailbox, argv[1]);
	networkingThread.detach();

	ClientThreadMain(clientData, masterMailbox);


	delete clientData;
	delete networkData;
	delete masterMailbox;

	return 0;
}
