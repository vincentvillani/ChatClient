/*
 * NetworkReadWriteBuffer.cpp
 *
 *  Created on: 21 Jan 2016
 *      Author: vincent
 */

#include "NetworkReadWriteBuffer.h"

#include <stdlib.h>

#define BUFFER_SIZE 1024 * 10

NetworkReadWriteBuffer::NetworkReadWriteBuffer(int socketHandle)
{
	this->socketHandle = socketHandle;
	readBuffer = new NetworkReadBuffer(BUFFER_SIZE, socketHandle);

}

NetworkReadWriteBuffer::~NetworkReadWriteBuffer()
{
	if(readBuffer != NULL)
		delete readBuffer;

	while(writeBufferQueue.size())
	{
		NetworkWriteBuffer* writeBuffer = writeBufferQueue.front();
		writeBufferQueue.pop();

		delete writeBuffer;
	}
}




