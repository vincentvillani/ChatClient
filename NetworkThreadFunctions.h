/*
 * NetworkThreadFunctions.h
 *
 *  Created on: 23 Jan 2016
 *      Author: vincent
 */

#ifndef NETWORKTHREADFUNCTIONS_H_
#define NETWORKTHREADFUNCTIONS_H_

#include "MasterMailbox.h"
#include "NetworkData.h"

class MasterMailbox;

void NetworkThreadMain(NetworkData* networkData, MasterMailbox* mailbox);

void NetworkThreadSendUsername(NetworkData* networkData, MasterMailbox* mailbox, std::string newUsername);

void NetworkThreadSendChatMessage(NetworkData* networkData, std::string currentMessage);


#endif /* NETWORKTHREADFUNCTIONS_H_ */
