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

class MasterMailbox;

void ClientThreadMain(ClientData* clientData, MasterMailbox* mailbox);

void ClientHandleUsernameChanged(ClientData* clientData);


#endif /* CLIENTTHREADFUNCTIONS_H_ */
