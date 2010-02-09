/*
 *  PCNetwork.h
 *  TootleNetwork
 *
 *  Created by Duane Bradbury on 03/02/2010.
 *  Copyright 2010 Tootle. All rights reserved.
 *
 */

#pragma once

#include "../TConnection.h"

namespace TLNetwork
{
	namespace Platform 
	{			
		// Wrapper pass-through routine to create a connection object of the correct platform specific type.
		// Eventually this should probably be added to a network manager which keeps track of the connection
		// as and when it is needed
		
		TPtr<TLNetwork::TConnection> CreateConnection();
	}
}
