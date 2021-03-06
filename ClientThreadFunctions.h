/*
 * ClientThreadFunctions.h
 *
 *  Created on: 23 Jan 2016
 *      Author: vincent
 */

#ifndef CLIENTTHREADFUNCTIONS_H_
#define CLIENTTHREADFUNCTIONS_H_

#include "ClientData.h"
#include "MasterMailbox.h"

#include <string>

class MasterMailbox;

void ClientThreadMain(ClientData* clientData, MasterMailbox* mailbox);

//void ClientHandleTerminalResize(ClientData* clientData, MasterMailbox* mailbox);

void ClientHandleUsernameChanged(ClientData* clientData);
void ClientHandleChatMessageReceived(ClientData* clientData, std::string username, std::string chatMessage);

void ClientHandleFailedToConnect(ClientData* clientData);
void ClientHandleDisconnect(ClientData* clientData);
void ClientHandleFailedReconnect(ClientData* clientData);


//void TerminalResizeHandler(MasterMailbox* mailbox, int sig);


#endif /* CLIENTTHREADFUNCTIONS_H_ */
