/*
 * UIFunctions.h
 *
 *  Created on: 30 Jan 2016
 *      Author: vincent
 */

#ifndef UIFUNCTIONS_H_
#define UIFUNCTIONS_H_

#include "ClientData.h"

#include <string>

//UI
//---------------------
void UISetup(ClientData* clientdata);
void UIShutdown();

void UIUpdate(ClientData* clientdata);

void UIAddMessage(ClientData* clientdata, UIMessage* newMessage);

void UIDraw(ClientData* clientdata);



#endif /* UIFUNCTIONS_H_ */
