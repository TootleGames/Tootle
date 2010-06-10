/*
 *  IPodNetwork.mm
 *  TootleNetwork
 *
 *  Created by Duane Bradbury on 03/02/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#import "IPodNetwork.h"

#import "IPodConnectionHttp.h"

TPtr<TLNetwork::TConnection> TLNetwork::Platform::CreateConnection()
{
	TPtr<TLNetwork::TConnection> pConnection = new Platform::TConnectionHttp();
	return  pConnection;
}

