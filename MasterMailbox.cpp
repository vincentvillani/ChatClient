/*
 * MasterMailbox.cpp
 *
 *  Created on: 23 Jan 2016
 *      Author: vincent
 */

#include "MasterMailbox.h"

MasterMailbox::MasterMailbox(ClientData* clientData, NetworkData* networkData)
{
	_clientData = clientData;
	_networkData = networkData;
}

MasterMailbox::~MasterMailbox()
{

}


void MasterMailbox::NetworkTellServerThreadUsernameUpdated()
{
	std::function<void()> functor = std::function<void()>(std::bind(ClientHandleUsernameChanged, _clientData));

	{
		std::lock_guard<std::mutex> workQueueLock(_clientData->mutex);
		_clientData->workQueue.push(functor);

	}

	_clientData->conditionVariable.notify_one();
}


void MasterMailbox::ClientSendChatMessage(std::string currentMessage)
{
	std::function<void()> functor = std::function<void()>(std::bind(NetworkThreadSendChatMessage, _networkData, currentMessage));
	{
		std::lock_guard<std::mutex> workQueueLock(_networkData->mutex);
		_networkData->workQueue.push(functor);
	}

	_networkData->conditionVariable.notify_one();
}

