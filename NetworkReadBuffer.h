/*
 * NetworkDataBuffer.h
 *
 *  Created on: 21 Jan 2016
 *      Author: vincent
 */

#ifndef NETWORKREADBUFFER_H_
#define NETWORKREADBUFFER_H_

#include <stdint.h>

class NetworkReadBuffer
{


public:

	uint32_t currentMessageSize;
	uint32_t bytesRead;
	uint32_t bufferSize;
	char* data;
	int socketHandle;


	NetworkReadBuffer(uint32_t bufferSize, int socketHandle);
	virtual ~NetworkReadBuffer();

	int remainingStorageCapacity();
	void grow();
};

#endif /* NETWORKREADBUFFER_H_ */
