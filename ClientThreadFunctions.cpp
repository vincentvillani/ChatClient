/*
 * ClientThreadFunctions.cpp
 *
 *  Created on: 23 Jan 2016
 *      Author: vincent
 */


#include "ClientThreadFunctions.h"

#include "Network.h"
#include "MasterMailbox.h"
#include "UIFunctions.h"

#include <string.h>
#include <chrono>
#include <functional>
#include <ncurses.h>
#include <signal.h>


#include <string.h>


static void DoAllWork(ClientData* clientData, MasterMailbox* mailbox)
{
	std::unique_lock<std::mutex> workQueueLock(clientData->mutex);

	while(clientData->workQueue.size())
	{
		std::function<void()> workItem = clientData->workQueue.front();
		clientData->workQueue.pop();

		//release the lock
		workQueueLock.unlock();

		//Do the work
		workItem();

		//Lock before checking the condition again
		workQueueLock.lock();
	}
}


void ClientThreadMain(ClientData* clientData, MasterMailbox* mailbox)
{

	UISetup(clientData, mailbox);

	UIDraw(clientData);


	while(clientData->clientThreadShouldContinue)
	{
		std::unique_lock<std::mutex> workQueueLock(clientData->mutex);

		bool workToDo = clientData->conditionVariable.wait_for(workQueueLock, std::chrono::milliseconds(10), [&]{return clientData->workQueue.size();});

		if(workToDo)
		{
			workQueueLock.unlock();
			DoAllWork(clientData, mailbox);
		}

		//Check for user input
		UIUpdate(clientData, mailbox);
	}



	UIShutdown();

}


void ClientHandleTerminalResize(ClientData* clientData, MasterMailbox* mailbox)
{
	UIResize(clientData, mailbox);
}



void ClientHandleUsernameChanged(ClientData* clientData)
{
	//printf("Client thread: Username sent to server!\n");
}


void ClientHandleChatMessageReceived(ClientData* clientData, std::string username, std::string chatMessage)
{
	UIMessage* newMessage = new UIMessage(username, chatMessage);

	UIAddMessage(clientData, newMessage);
	UIDraw(clientData);
}



void ClientHandleFailedToConnect(ClientData* clientData)
{
	std::string username = "Notification";

	std::string message = "Unable to connect to the server. Please try to connect later. Press any key to exit the program...";

	UIMessage* newMessage = new UIMessage(username, message);

	UIAddMessage(clientData, newMessage);
	UIDraw(clientData);

	//Shutdown the thread
	clientData->clientThreadShouldContinue = false;

	//Wait indefinetly
	timeout(-1);

	//wait for any key to be pressed
	getch();

	//Don't block for the next update call, so the program can exit <- Could just move the work UIUpdate call to above the work item queue calls
	timeout(0);
}


void ClientHandleDisconnect(ClientData* clientData)
{
	std::string username = "Notification";
	std::string message = "You have been disconnected from the server. Attempting to reconnect...";

	UIMessage* newMessage = new UIMessage(username, message);

	UIAddMessage(clientData, newMessage);
	UIDraw(clientData);
}


void ClientHandleFailedReconnect(ClientData* clientData)
{
	std::string username = "Notification";
	std::string message = "Unable to reconnect to the server. Please try to reconnect later. Press any key to exit the program...";

	UIMessage* newMessage = new UIMessage(username, message);

	UIAddMessage(clientData, newMessage);
	UIDraw(clientData);

	//Shutdown the thread
	clientData->clientThreadShouldContinue = false;

	//Wait indefinetly
	timeout(-1);

	//wait for any key to be pressed
	getch();

	//Don't block for the next update call, so the program can exit <- Could just move the work UIUpdate call to above the work item queue calls
	timeout(0);


}


void TerminalResizeHandler(MasterMailbox* mailbox, int sig)
{
	mailbox->ClientThreadUIShouldResize();
}

