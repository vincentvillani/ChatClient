/*
 * UIFunctions.h
 *
 *  Created on: 30 Jan 2016
 *      Author: vincent
 */

#ifndef UIFUNCTIONS_H_
#define UIFUNCTIONS_H_

#include "ClientData.h"
#include "MasterMailbox.h"

#include <string>

//UI
//---------------------
void UISetup(ClientData* clientdata, MasterMailbox* mailbox);
void UIShutdown();

void UIUpdate(ClientData* clientdata, MasterMailbox* mailbox);

void UIAddMessage(ClientData* clientdata, UIMessage* newMessage);

void UIDraw(ClientData* clientdata);

void UIResize(ClientData* clientdata, MasterMailbox* mailbox);



#endif /* UIFUNCTIONS_H_ */
