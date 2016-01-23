/*
 * NetworkDataBuffer.cpp
 *
 *  Created on: 21 Jan 2016
 *      Author: vincent
 */

#include <stdlib.h>
#include <string.h>
#include "NetworkReadBuffer.h"

NetworkReadBuffer::NetworkReadBuffer(uint32_t bufferSize, int socketHandle)
{
	currentMessageSize = 0;
	bytesRead = 0;
	data = (char*)malloc(bufferSize);
	this->bufferSize = bufferSize;
	this->socketHandle = socketHandle;

}

NetworkReadBuffer::~NetworkReadBuffer()
{
	if(data != NULL)
		free(data);
}


int NetworkReadBuffer::remainingStorageCapacity()
{
	return bufferSize - bytesRead;
}



void NetworkReadBuffer::grow()
{
	size_t newBufferSize = bufferSize * 1.5f;
	char* buffer = (char*)malloc(newBufferSize);

	//Copy the old data across, only the data we actually care about
	memcpy(buffer, data, bytesRead);

	free(data);
	bufferSize = newBufferSize;
	data = buffer;
}

