/*------------------------------------------------------
	
	Network functions

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLDebug.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TBinaryTree.h>

namespace TLNetwork
{
	class TConnection;
	class TSocketServer;
	
	// Wrapper pass-through routine to create a connection object of the correct platform specific type.
	// Eventually this should probably be added to a network manager which keeps track of the connection
	// as and when it is needed
	TPtr<TLNetwork::TConnection> CreateConnection();
	
	//	gr: more temporaryness (this doesn't derive from TConnection yet)
	TPtr<TLNetwork::TSocketServer> CreateServerSocket(TBinaryTree& ServerInfo);
}

