#include "TLNetwork.h"
#include <TootleCore/TLCore.h>
#include <TootleCore/TCoreManager.h>
#include "TSocketServer.h"
#include "TConnection.h"


namespace TLNetwork
{
	namespace Platform
	{
		// Wrapper pass-through routine to create a connection object of the correct platform specific type.
		// Eventually this should probably be added to a network manager which keeps track of the connection
		// as and when it is needed
		// Creates the appropriate connection class based on the platform being built
		TPtr<TLNetwork::TConnection> CreateConnection();
	}


	TRef		g_PeerCounter = TRef_Static(P,r,ZERO,ZERO,ZERO);	//	unique ref for peers, starting at Pr001 (first use is incremented)
}



//---------------------------------------------------
//	
//---------------------------------------------------
TPtr<TLNetwork::TConnection> TLNetwork::CreateConnection()
{
	return Platform::CreateConnection();
}



//---------------------------------------------------
//	gr: more temporaryness (this doesn't derive from TConnection yet)
//---------------------------------------------------
TPtr<TLNetwork::TSocketServer> TLNetwork::CreateServerSocket(TBinaryTree& ServerInfo)
{
	return new TSocketServer( ServerInfo );
}


//---------------------------------------------------
//	
//---------------------------------------------------
TLNetwork::TPeer::TPeer() :
	m_PeerRef	( g_PeerCounter.Increment() )
{
}


//---------------------------------------------------
//	append description of this Peer to the string
//---------------------------------------------------
void TLNetwork::TPeer::GetDescription(TString& String) const
{
	String << GetPeerRef();
}




	
//----------------------------------------------------------------------------//
//	
//----------------------------------------------------------------------------//
TLNetwork::TPacket::TPacket(TRefRef PacketRef,const TBinary& Data,const TPeer& ToPeer,const TPeer& FromPeer) :
	m_PacketRef	( PacketRef ),
	m_Data		( Data ),
	m_SendSmall	( false ),
	m_DataSent	( 0 ),
	m_pFromPeer	( &FromPeer ),
	m_pToPeer	( &ToPeer )
{
}

//----------------------------------------------------------------------------//
//	increment amount of data sent. 
//----------------------------------------------------------------------------//
void TLNetwork::TPacket::OnDataSent(u16 BytesSent)		
{	
	//	increment sent amount
	m_DataSent += BytesSent;

	//	check we we haven't sent more than we have to send...
	if ( m_DataSent > m_Data.GetSize() )
	{
		TDebugString Debug_String;
		Debug_String << "Apparently we have sent more data (" << m_DataSent << ") than we have to send in the first place (" << m_Data.GetSize() << ").";
		TLDebug_Break( Debug_String );

		//	continguency plan.
		m_DataSent = m_Data.GetSize();
	}
}
