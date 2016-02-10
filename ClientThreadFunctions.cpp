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

	UISetup(clientData);

	UIDraw(clientData);


	while(true)
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
