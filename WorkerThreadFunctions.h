/*
 * WorkerThreadFunctions.h
 *
 *  Created on: 13 Jan 2016
 *      Author: vincent
 */

#ifndef WORKERTHREADFUNCTIONS_H_
#define WORKERTHREADFUNCTIONS_H_

#include "Socket.h"
#include <stdint.h>

void SendMessageToServer(Socket* serverSocket, char* message, uint32_t messageLength);



#endif /* WORKERTHREADFUNCTIONS_H_ */
