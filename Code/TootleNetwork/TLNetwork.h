/*------------------------------------------------------
	
	Network functions

-------------------------------------------------------*/
#pragma once

#include "TConnection.h"

namespace TLNetwork
{
	// Wrapper pass-through routine to create a connection object of the correct platform specific type.
	// Eventually this should probably be added to a network manager which keeps track of the connection
	// as and when it is needed
	TPtr<TLNetwork::TConnection> CreateConnection();
}


