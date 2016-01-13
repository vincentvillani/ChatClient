/*
 * ClientThreadPool.cpp
 *
 *  Created on: 13 Jan 2016
 *      Author: vincent
 */

#include "ClientThreadPool.h"

ClientThreadPool::ClientThreadPool()
{

}

ClientThreadPool::~ClientThreadPool()
{
}


//std::function<void(Socket*, char*, uint32_t
void ClientThreadPool::addToWorkBuffer(std::function<void()> workUnit)
{
	_workBuffer.push_back(workUnit);
}


void ClientThreadPool::debugTest()
{
	auto workItem = _workBuffer[0];
	workItem();
}
