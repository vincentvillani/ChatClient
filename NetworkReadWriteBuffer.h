/*
 * NetworkReadWriteBuffer.h
 *
 *  Created on: 21 Jan 2016
 *      Author: vincent
 */
#ifndef NETWORKREADWRITEBUFFER_H_
#define NETWORKREADWRITEBUFFER_H_

#include "NetworkReadBuffer.h"
#include "NetworkWriteBuffer.h"

#include <queue>



class NetworkReadWriteBuffer
{
private:



public:
	NetworkReadWriteBuffer(int socketHandle);
	virtual ~NetworkReadWriteBuffer();

	NetworkReadBuffer* readBuffer;
	std::queue<NetworkWriteBuffer*> writeBufferQueue;
	int socketHandle;

};

#endif /* NETWORKREADWRITEBUFFER_H_ */
