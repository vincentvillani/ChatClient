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

void UIAddString(ClientData* clientdata, std::string newString);

void UIDraw(ClientData* clientdata);



#endif /* UIFUNCTIONS_H_ */
