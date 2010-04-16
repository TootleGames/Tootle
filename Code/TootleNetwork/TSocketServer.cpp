#include "TSocketServer.h"
#include <TootleCore/TLCore.h>
#include <TootleCore/TCoreManager.h>


//	gr: this is a bit temporary. todo: replace with a generic 
//		TLNetwork::Platform::CreateSocket() function
#if defined(TL_TARGET_PC)
	#include "PC/PCSocket.h"
#elif defined(TL_TARGET_MAC)
	#include "Mac/MacSocket.h"
#elif defined(TL_TARGET_IPOD)
	#include "IPod/IPodSocket.h"
#endif



//---------------------------------------------------
//	recieved some packet data - append it to our packet
//---------------------------------------------------
void TLNetwork::TClient::OnRecievedPacketPart(TArray<u8>& Data,const TPeer& ServerPeer)
{
	//	alloc new packet if we don't have one
	if ( !m_pRecievingPacket )
	{
		m_pRecievingPacket = new TPacket( TRef(), TBinary(), ServerPeer, *this );
	}

	//	append the new data to the packet's data
	m_pRecievingPacket->GetData().WriteData( Data );
}





TLNetwork::TSocketServer::TSocketServer(TBinaryTree& InitData) :
	m_Port			( 0 )
{
	if ( !InitData.ImportData("Port", m_Port ) )
		TLDebug_Break("Port expected");

	InitData.ImportData("Protocol", m_Protocol );
	if ( m_Protocol != "TCP" && m_Protocol != "UDP" )
	{
		TDebugString Debug_String;
		Debug_String << m_Protocol << " is not a valid socket protocol. Must be UDP or TCP";
		TLDebug_Break( Debug_String );
		m_Protocol.SetInvalid();
	}
}

TLNetwork::TSocketServer::~TSocketServer()
{
}

SyncBool TLNetwork::TSocketServer::Initialise(TRef& ErrorRef)
{
	//	require a port
	if ( m_Port == 0 )
	{
		ErrorRef = "NoPort";
		return SyncFalse;
	}

	//	invalid protocol
	if ( m_Protocol != "TCP" && m_Protocol != "UDP" )
	{
		ErrorRef = "NoProtocol";
		return SyncFalse;
	}

    //	create socket for the server
	if ( !m_pListeningSocket )
		m_pListeningSocket = new Platform::Socket;

	SyncBool Result = m_pListeningSocket->Initialise( m_Port, m_Protocol );
	if ( Result != SyncTrue )
	{
		ErrorRef = (Result == SyncFalse) ? TRef("ListenFailed") : TRef();
        return Result;
    }

	//	all initialised! so subscribe to updates
	this->SubscribeTo( TLCore::g_pCoreManager );

	return SyncTrue;
}


//---------------------------------------------------------
//	get update messages
//---------------------------------------------------------
void TLNetwork::TSocketServer::ProcessMessage(TLMessaging::TMessage& Message)
{
	if ( Message.GetMessageRef() == TLCore::UpdateRef )
	{
		Update();
	}
}


//---------------------------------------------------------
//	check for new connections. returns false if there's an error with winsock and we need to shutdown
//---------------------------------------------------------
bool TLNetwork::TSocketServer::CheckForNewConnections()
{
	//	get new peer sockets from the listening socket
	TPtrArray<TPeerSocket> NewPeers;
	if ( !m_pListeningSocket->GetNewPeers( NewPeers ) )
		return false;

	//	setup new clients for each new connection
	for ( u32 p=0;	p<NewPeers.GetSize();	p++ )
	{
		TPtr<TPeerSocket>& pPeerSocket = NewPeers[p];

		TPtr<TClient> pPeer = new TClient( pPeerSocket );
		OnPeerConnected( pPeer );
	}

	//	winsock okay
	return true;
}


//---------------------------------------------------------
//	regular update. return false if error with winsock and need to shutdown
//---------------------------------------------------------
bool TLNetwork::TSocketServer::Update()
{
	//	look out for new connections
	if ( !CheckForNewConnections() )
	{
		//	winsock error, shutdown!
		return false;
	}

	//	look out for data on Peers
	//	gr: done in reverse in case any peers are disconnected mid-loop
	//	gr: will this have any knock-on-effects with processing order?
	for ( s32 p=m_Clients.GetLastIndex();	p>=0;	p-- )
	{
		TClient& PeerSocket = *m_Clients[p];
	
		//	recieve new data from Peer
		TConnState PeerConnState = RecieveData( PeerSocket );

		//	send new data to peer (if still okay)
		if ( PeerConnState == ConnState_Connected )
			PeerConnState = SendData( PeerSocket );

		//	if error then disconnect peer
		if ( PeerConnState != ConnState_Connected )
		{
			OnPeerDisconnected( PeerSocket, (PeerConnState==ConnState_DisconnectedGood) );
			continue;
		}
	}

	return true;
}


//---------------------------------------------------------
//	recieve new data from Peer. 
//---------------------------------------------------------
TLNetwork::TSocketServer::TConnState TLNetwork::TSocketServer::RecieveData(TClient& Peer)	
{
	TFixedArray<u8,2048> Buffer;
	TRef Error;
	SyncBool Result = Peer.GetSocket().RecieveData( Buffer, Error );

	//	error
	if ( Result == SyncFalse )
	{
		//	connection has been gracefully closed
		return Error.IsValid() ? ConnState_DisconnectedBad : ConnState_DisconnectedGood;
	}

	//	nothing recieved
	if ( Buffer.GetSize() == 0 )
		return ConnState_Connected;

	//	push the data into the Peer's buffer
	Peer.OnRecievedPacketPart( Buffer, m_ListeningPeer );

	//	no more data, so notify that we've filled up the data for this Peer
	//	and have a packet to send
	if ( Result == SyncTrue )
	{
		OnRecievedPacket( Peer );
	}

	return ConnState_Connected;
}


//----------------------------------------------------------------------------//
//	send queued packets on peer (sending to peer)
//----------------------------------------------------------------------------//
TLNetwork::TSocketServer::TConnState TLNetwork::TSocketServer::SendData(TClient& Peer)
{
	//	send the queued packets on the peer
	TPtrArray<TPacket>& PacketQueue = Peer.GetSendPacketQueue();
	while ( PacketQueue.GetSize() > 0 )
	{
		TPacket& NextPacket = *PacketQueue[0];

		//	send the packet
		TConnState ConnState = ConnState_Invalid;
		SyncBool SendResult = SendPacket( Peer, NextPacket, ConnState );

		//	didn't finish (more to send!), finish
		if ( SendResult == SyncWait )
		{
			if ( ConnState != ConnState_Connected )
				TLDebug_Break("Expected connection state to be connected still");
			return ConnState;
		}

		//	failed
		if ( SendResult == SyncFalse )
			return ConnState;

		//	check conn state is valid
		if ( ConnState != ConnState_Connected )
			TLDebug_Break("Expected connection state to be connected still");

		//	success, notify packet has been sent
		if ( HasSubscribers() )
		{
			TLMessaging::TMessage Message( TRef_Static(O,n,S,e,n) );
			Message.Write( NextPacket.GetPacketRef() );
			Message.ExportData( TRef_Static4(P,e,e,r), Peer.GetPeerRef() );
			PublishMessage( Message );
		}

		//	remove this packet from the send queue
		PacketQueue.RemoveAt(0);
	}

	//	no errors
	return ConnState_Connected;
}


//----------------------------------------------------------------------------//
//	send this packet to the peer. returns true if sent, false if failed, wait if only partially sent
//----------------------------------------------------------------------------//
SyncBool TLNetwork::TSocketServer::SendPacket(TClient& Client,TPacket& Packet,TLNetwork::TSocketServer::TConnState& ConnState)
{
	//	check we're trying to send the right packet from the right peer
	if ( Packet.GetToPeer() != Client )
	{
		if ( !TLDebug_Break("Peer/packet mis-match when sending packet from server") )
			return SyncFalse;
	}

	//	resume packet sending
	const u8* pData = Packet.GetDataToSend();
	u16 DataSize = Packet.GetDataToSendSize();

	//	limit amount to send
	TLMaths::Limit<u16>( DataSize, 0, Packet.GetSendLimit() );

	//	send data
	u32 DataSent;
	bool TooLarge;
	TRef Error;
	if ( !Client.GetSocket().SendData( pData, DataSize, DataSent, TooLarge, Error ) )
	{
		//	system error
		ConnState = Error.IsValid() ? ConnState_DisconnectedBad : ConnState_DisconnectedGood;
		return SyncFalse;
	}

	//	packet was too large to send! re-configure the packet
	if ( TooLarge )
		Packet.OnDataTooLarge();

	//	increment "how many bytes sent"
	if ( DataSent > 0xffff )
	{
		TLDebug_Break("Sent more data than fits in 16bits!");
	}
	Packet.OnDataSent( (u16)DataSent );

	//	set connection result
	ConnState = ConnState_Connected;

	//	return whether there is more to send or not
	if ( Packet.GetDataToSendSize() > 0 )
		return SyncWait;
	else
		return SyncTrue;
}


//---------------------------------------------------------
//	send this packet to this peer
//---------------------------------------------------------
bool TLNetwork::TSocketServer::SendToPeer(const TBinary& PacketData,TRefRef PeerRef,TRefRef PacketRef)
{
	//	find peer's socket
	TClient* pPeerSocket = GetClient( PeerRef );
	if ( !pPeerSocket )
		return false;

	//	queue up on peer
	return pPeerSocket->SendPacket( PacketData, m_ListeningPeer, PacketRef );
}

//---------------------------------------------------------
//	peer connected - notify subscribers
//---------------------------------------------------------
void TLNetwork::TSocketServer::OnPeerConnected(TPtr<TLNetwork::TClient>& pPeer)	
{
	if ( !pPeer )
	{
		TLDebug_Break("Peer expected");
		return;
	}

	//	add to list of peers
	s32 PeerIndex = m_Clients.Add( pPeer );

	//	if we failed to add the peer, then don't send notification. By design, if this fails, when 
	//	we leave the function the TPeerSocket should cleanup (when last Ptr releases) and destroy the socket/connection
	//	and disconnected the peer meaning we don't have to do any cleanup.
	if ( PeerIndex == -1 )
		return;

	//	gr: show warning as we assume someone generally wants to know about peer connection.
	//		if we have cases where we don't, then feel free to remove this warning
	if ( !HasSubscribers() )
	{
		TLDebug_Print("Warning: peer connected, but no subscriber[s] to recieve notification!");
	}

	//	notify on connection
	TLMessaging::TMessage Message( TRef_Static(P,R,C,o,n) );
	Message.ExportData( TRef_Static4(P,e,e,r), pPeer->GetPeerRef() );
	PublishMessage( Message );
}


//---------------------------------------------------------
//	peer disconnected - notify subscribers
//---------------------------------------------------------
void TLNetwork::TSocketServer::OnPeerDisconnected(TRef PeerRef,bool Gracefully)	
{
	//	find peer
	s32 PeerIndex = m_Clients.FindIndex( PeerRef );
	if ( PeerIndex == -1 )
	{
		TLDebug_Break("Peer expected");
		return;
	}

	//	remove from peer list (should delete unless anything has ptr's still to peer)
	m_Clients.RemoveAt( PeerIndex );

	//	send out notification
	TLMessaging::TMessage Message( TRef_Static(P,R,D,i,s) );
	Message.ExportData( TRef_Static4(P,e,e,r), PeerRef );
	Message.ExportData( TRef_Static(G,r,a,c,e), Gracefully );
	PublishMessage( Message );
}


//---------------------------------------------------------
//	finished recieving a block of data from a client
//---------------------------------------------------------
void TLNetwork::TSocketServer::OnRecievedPacket(TLNetwork::TClient& Peer)
{
	//	take packet off the peer (note: not a reference!)
	TPtr<TPacket> pRecievingPacket = Peer.GetRecievingPacket();
	if ( !pRecievingPacket )
	{
		TLDebug_Break("packet expected on peer after recieving data...");
		return;
	}

	//	remove packet from peer
	Peer.FlushRecievingPacket();

	//	create packet message to send out to subscribers
	TLMessaging::TMessage PacketMessage( TRef_Static(O,n,P,c,k) );
	PacketMessage.ExportDataData( TRef_Static4(D,a,t,a), pRecievingPacket->GetData() );
	PacketMessage.ExportData( TRef_Static4(P,e,e,r), Peer.GetPeerRef() );

	//	send out this packet
	PublishMessage( PacketMessage );
}



TLNetwork::TClient::TClient(TPtr<TPeerSocket>& pPeerSocket) :
	m_pSocket	( pPeerSocket )
{
	if ( !pPeerSocket )
		TLDebug_Break("Socket expected");

	//	get address
	if ( m_pSocket )
		m_PeerAddress = m_pSocket->GetAddress();
}


//-----------------------------------------------------------
//	append description of this Peer to the string
//-----------------------------------------------------------
void TLNetwork::TClient::GetDescription(TString& String) const
{
	TPeer::GetDescription( String );
	String << '(' << m_PeerAddress << ')';
}
