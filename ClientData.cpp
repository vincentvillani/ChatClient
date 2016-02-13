/*
 * ChatClient.cpp
 *
 *  Created on: 13 Jan 2016
 *      Author: vincent
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "ClientData.h"
#include "Network.h"



ClientData::ClientData(std::string username)
{
	this->username = username;
	this->clientThreadShouldContinue = true;
}

ClientData::~ClientData()
{
}

