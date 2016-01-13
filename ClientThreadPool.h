/*
 * ClientThreadPool.h
 *
 *  Created on: 13 Jan 2016
 *      Author: vincent
 */

#ifndef CLIENTTHREADPOOL_H_
#define CLIENTTHREADPOOL_H_

#include <vector>
#include <functional>
#include <stdint.h>
#include <mutex>
#include <condition_variable>

#include "Socket.h"

class ClientThreadPool
{
	std::vector<std::function<void()>> _workBuffer;
	std::mutex _workMutex;
	std::condition_variable _workBufferCV;

public:

	ClientThreadPool();
	virtual ~ClientThreadPool();

	void addToWorkBuffer(std::function<void()> workUnit);
	void debugTest();
};

#endif /* CLIENTTHREADPOOL_H_ */
