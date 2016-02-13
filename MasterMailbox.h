/*
 * MasterMailbox.h
 *
 *  Created on: 23 Jan 2016
 *      Author: vincent
 */

#ifndef MASTERMAILBOX_H_
#define MASTERMAILBOX_H_

#include "NetworkData.h"
#include "ClientData.h"

#include "ClientThreadFunctions.h"
#include "NetworkThreadFunctions.h"

#include <string>


class MasterMailbox
{

	ClientData* _clientData;
	NetworkData* _networkData;

public:
	MasterMailbox(ClientData* clientData, NetworkData* networkData);
	virtual ~MasterMailbox();

	void NetworkTellServerThreadUsernameUpdated();

	void ClientThreadSendChatMessage(std::string currentMessage);

	void NetworkThreadFailedToConnectToServer();
	void NetworkThreadDisconnectOccured();
	void NetworkThreadUnableToReconnect(); //Unable to reconnect to the server so shut down the program

	void NetworkThreadChatMessageReceived(std::string username, std::string chatMessage);

};

#endif /* MASTERMAILBOX_H_ */
