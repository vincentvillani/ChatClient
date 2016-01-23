/*
 * WorkQueue.h
 *
 *  Created on: 23 Jan 2016
 *      Author: vincent
 */

#ifndef WORKQUEUE_H_
#define WORKQUEUE_H_

#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

class WorkQueue
{
public:

	std::mutex mutex;
	std::condition_variable conditionVariable;
	std::queue<std::function<void()>> workQueue;

	WorkQueue();
	virtual ~WorkQueue();


};

#endif /* WORKQUEUE_H_ */
