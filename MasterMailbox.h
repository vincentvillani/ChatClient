/*
 * MasterMailbox.h
 *
 *  Created on: 23 Jan 2016
 *      Author: vincent
 */

#ifndef MASTERMAILBOX_H_
#define MASTERMAILBOX_H_

#include "NetworkData.h"
#include "ClientData.h"

#include "ClientThreadFunctions.h"
#include "NetworkThreadFunctions.h"


class MasterMailbox
{

	ClientData* _clientData;
	NetworkData* _networkData;

public:
	MasterMailbox(ClientData* clientData, NetworkData* networkData);
	virtual ~MasterMailbox();

	void NetworkTellServerThreadUsernameUpdated();

};

#endif /* MASTERMAILBOX_H_ */
