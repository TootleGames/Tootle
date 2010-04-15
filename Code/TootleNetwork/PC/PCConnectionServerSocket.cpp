#include "PCConnectionServerSocket.h"
#include <TootleCore/TLCore.h>
#include <TootleCore/TCoreManager.h>



TLNetwork::TAddress GetSockAddrAddress(const sockaddr_storage& SocketAddress)
{
	//	IPv6 not supported yet
	if ( SocketAddress.ss_family == AF_INET6 || SocketAddress.ss_family != AF_INET )
	{
		TLDebug_Break("IPv6 not yet supported.");
		return TLNetwork::TAddress();
	}

	//	convert to IPv4 sturct
    sockaddr_in& SocketAddress4 = (sockaddr_in&)SocketAddress;

	//	TCP -> hostname conversion (big vs little endian)
	SocketAddress4.sin_port = ntohs( SocketAddress4.sin_port );
	IN_ADDR& InAddr = SocketAddress4.sin_addr;
	InAddr.S_un.S_addr = ntohl( InAddr.S_un.S_addr );

	//	set address
	TLNetwork::TAddress Result( Type4<u8>( InAddr.S_un.S_un_b.s_b1, InAddr.S_un.S_un_b.s_b2, InAddr.S_un.S_un_b.s_b3, InAddr.S_un.S_un_b.s_b4 ), SocketAddress4.sin_port );

	//	gr: bodge for local addresses. Not sure this is right...
	if ( Result.GetIP() == Type4<u8>(0,0,0,0) )
		Result = TLNetwork::TAddress( Type4<u8>(127,0,0,1), Result.GetPort() );

	TDebugString Debug_String;
	Debug_String << "Resolved socket to " << Result;
	TLDebug_Print( Debug_String );

	return Result;
}


TLNetwork::TAddress GetBoundSocketAddress(SOCKET& Socket)
{
	//	get address
	sockaddr_storage SocketAddress;
	int SizeOfSocketAddress = sizeof(SocketAddress);

	//	get peer's name from their socket (a socket we're connected to)
	int Result = getsockname( Socket, (sockaddr*)&SocketAddress, &SizeOfSocketAddress );
	if ( Result == SOCKET_ERROR )
	{
		//	check the error
		int Error = WSAGetLastError();
		return TLNetwork::TAddress();
	}

	return GetSockAddrAddress( SocketAddress );
}

TLNetwork::TAddress GetConnectedSocketAddress(SOCKET& Socket)
{
	//	get address
	sockaddr_storage SocketAddress;
	int SizeOfSocketAddress = sizeof(SocketAddress);

	//	get peer's name from their socket (a socket we're connected to)
	int Result = getpeername( Socket, (sockaddr*)&SocketAddress, &SizeOfSocketAddress );
	if ( Result == SOCKET_ERROR )
	{
		//	check the error
		int Error = WSAGetLastError();
		return TLNetwork::TAddress();
	}

	return GetSockAddrAddress( SocketAddress );
}




TLNetwork::TPeerSocket::~TPeerSocket()
{
    // shutdown the connection since we're done
	int Result = shutdown( m_Socket, SD_SEND );
    
	if ( Result == SOCKET_ERROR )
	{
		TLDebug_Print("shutdown failed");
    }

    // cleanup
    closesocket( m_Socket );
	m_Socket = INVALID_SOCKET;
}



//---------------------------------------------------
//	recieved some packet data - append it to our packet
//---------------------------------------------------
void TLNetwork::TPeerSocket::OnRecievedPacketPart(TArray<u8>& Data,const TPeer& ServerPeer)
{
	//	alloc new packet if we don't have one
	if ( !m_pRecievingPacket )
	{
		m_pRecievingPacket = new TPacket( TRef(), TBinary(), ServerPeer, *this );
	}

	//	append the new data to the packet's data
	m_pRecievingPacket->GetData().WriteData( Data );
}




TLNetwork::TSocket::TSocket(const struct addrinfo& AddrInfo) :
	m_Socket	( INVALID_SOCKET ),
	m_IsOpen	( false )
{
	//	create socket
	m_Socket = socket( AddrInfo.ai_family, AddrInfo.ai_socktype, AddrInfo.ai_protocol );

	//	set the blocking mode (non-zero == on)
	u_long iMode = 1;
	int Result = ioctlsocket( m_Socket, FIONBIO, &iMode );
	if ( Result == SOCKET_ERROR )
	{
		//	check the error
		int Error = WSAGetLastError();

		//	error setting mode so shutdown (we don't want blocking sockets)
		Shutdown();
		return;
	}
}


TLNetwork::TSocket::~TSocket()
{
	Shutdown();
}


void TLNetwork::TSocket::Shutdown()
{
	//	close socket if it's open
	if ( m_IsOpen )
	{
		closesocket( m_Socket );
		m_IsOpen = false;
	}

	//	shutdown the socket
	if ( m_Socket != INVALID_SOCKET )
	{
		shutdown( m_Socket, SD_BOTH );

		//	invalidate socket
		m_Socket = INVALID_SOCKET;
	}
}


bool TLNetwork::TSocket::Bind(const struct addrinfo& AddrInfo)
{
	if ( !IsValid() )
		return false;

	//	already open?
	if ( m_IsOpen )
	{
		TLDebug_Break("trying to bind socket which is already open");
		return false;
	}

	int Result = bind( m_Socket, AddrInfo.ai_addr, AddrInfo.ai_addrlen );
    if ( Result == SOCKET_ERROR )
	{
		//	check the error
		int Error = WSAGetLastError();
		return false;
	}

	//	will need closing now
	m_IsOpen = true;
	
	//	get address
	m_Address = GetBoundSocketAddress( m_Socket );

	//	failed to resolve address of socket...
	if ( !m_Address.IsValid() )
		return false;


	return true;
}


TLNetwork::TTempAddrInfo::TTempAddrInfo(TRefRef Protocol,u16 Port) :
	m_pAddrInfo	( NULL )
{
	//	setup the addrinfo we want
	struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));

	//	gr: INET is ipv4
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;

	if ( Protocol == "TCP" )
	{
		//	stream is TCP
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
	}
	else // UDP
	{
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
	}

	//	need port in a string
	TTempString PortString;
	PortString << Port;

	//	Resolve the local address and port
	TFixedArray<char,100> AnsiPortString;
	PortString.GetAnsi( AnsiPortString );

	int Error = getaddrinfo( NULL, AnsiPortString.GetData(), &hints, &m_pAddrInfo );
    if ( Error != 0 )
	{
		//	failed
		return;
	}
}

TLNetwork::TTempAddrInfo::~TTempAddrInfo()
{
	if ( m_pAddrInfo )
	{
        freeaddrinfo( m_pAddrInfo );
		m_pAddrInfo = NULL;
	}
}




TLNetwork::TWinsockManager::TWinsockManager() :
	m_DataValid	( false )
{
	//	initialise
	int Result = WSAStartup( MAKEWORD(2,2), &m_Data );
	
	//	no error!
	if ( Result == 0 )
	{
		m_DataValid = true;
	}
}


TLNetwork::TWinsockManager::~TWinsockManager()
{
	//	winsock needs cleaning up
	if ( m_DataValid )
	{
		WSACleanup();
		m_DataValid = false;
	}
}



TLNetwork::TConnectionServerSocket::TConnectionServerSocket(TBinaryTree& InitData) :
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

TLNetwork::TConnectionServerSocket::~TConnectionServerSocket()
{
}

SyncBool TLNetwork::TConnectionServerSocket::Initialise(TRef& ErrorRef)
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

	// Initialize Winsock (use existing winsock if it exists otherwise we alloc a new one whilst we setup)
	TPtr<TWinsockManager> pWinsock = m_pWinsock;
	if ( !pWinsock )
	{
		pWinsock = new TWinsockManager;
		if ( !pWinsock->IsInitialised() )
		{
			ErrorRef = "WinSockFailed";
			return SyncFalse;
		}
	}

    //	Resolve the local address and port
	TTempAddrInfo ServerAddrInfo( m_Protocol, m_Port );
	if ( !ServerAddrInfo.IsValid() )
	{
		ErrorRef = "GetAddrFailed";
        return SyncFalse;
    }

    //	create socket for the server
	TPtr<TSocket> pListeningSocket = new TSocket( ServerAddrInfo.GetData() );
	if ( !pListeningSocket->IsValid() )
	{
		ErrorRef = "ListenFailed";
        return SyncFalse;
    }

    //	bind socket to get connections
	if ( !pListeningSocket->Bind( ServerAddrInfo.GetData() ) )
	{
		ErrorRef = "BindFailed";
		return SyncFalse;
	}

	//	look out for a socket connection request
	int Result = listen( pListeningSocket->GetData(), SOMAXCONN );
	if ( Result == SOCKET_ERROR )
	{
		//	check the error
		int Error = WSAGetLastError();
		ErrorRef = "ListenFailed";
		return SyncFalse;
	}

	//	save the winsock initialisation
	m_pWinsock = pWinsock;
	m_pListeningSocket = pListeningSocket;

	//	all initialised! so subscribe to updates
	this->SubscribeTo( TLCore::g_pCoreManager );

	return SyncTrue;
}


//---------------------------------------------------------
//	get update messages
//---------------------------------------------------------
void TLNetwork::TConnectionServerSocket::ProcessMessage(TLMessaging::TMessage& Message)
{
	if ( Message.GetMessageRef() == TLCore::UpdateRef )
	{
		Update();
	}
}


//---------------------------------------------------------
//	check for new connections. returns false if there's an error with winsock and we need to shutdown
//---------------------------------------------------------
bool TLNetwork::TConnectionServerSocket::CheckForNewConnections()
{
	// Accept a Peer socket
	SOCKET PeerSocket = accept( m_pListeningSocket->GetData(), NULL, NULL );
	
	//	no incoming connection
	if ( PeerSocket == INVALID_SOCKET )
	{
		//	check the error
		int Error = WSAGetLastError();

		//	if would block, then that means if it was a blocking socket we'd stick here until
		//	someone did try and connect. so to us we've just not had a connection request 
		if ( Error == WSAEWOULDBLOCK )
			return true;

		//	some error! kill winsock!
		return false;
	}

	//	new connection - create new Peer socket
	TPtr<TPeerSocket> pPeer = new TPeerSocket( PeerSocket );
	OnPeerConnected( pPeer );

	//	winsock okay
	return true;
}


//---------------------------------------------------------
//	regular update. return false if error with winsock and need to shutdown
//---------------------------------------------------------
bool TLNetwork::TConnectionServerSocket::Update()
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
	for ( s32 p=m_Peers.GetLastIndex();	p>=0;	p-- )
	{
		TPeerSocket& PeerSocket = *m_Peers[p];
	
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
TLNetwork::TConnectionServerSocket::TConnState TLNetwork::TConnectionServerSocket::RecieveData(TPeerSocket& Peer)	
{
	//	gr: note; for udp, we lose excess data so we need to peek data!
	//	check if there's any data to be recieved from the Peer
	TFixedArray<u8,2048> Buffer;
	Buffer.SetAllocSize( 2048 );	//	gr: not required, but added in case we change the buffer type and forget to add it
	int RecieveFlags = 0x0;
	int Result = recv( Peer.GetSocket(), (char*)Buffer.GetData(), Buffer.GetAllocSize(), RecieveFlags );

	//	connection has been gracefully closed
	if ( Result == 0 )
		return ConnState_DisconnectedGood;

	//	this is true if we recieved data up to the limit of our buffer and we need to get some more
	bool MoreData = false;

	//	check for error
	if ( Result == SOCKET_ERROR )
	{
		int LastError = WSAGetLastError();

		switch ( LastError )
		{
			//	not an error, just nothing to receive
			case WSAEWOULDBLOCK:
				return ConnState_Connected;

			//	buffer was filled, but there's more to come
			case WSAEMSGSIZE:
				MoreData = true;
				//	set the result to the amount of data recieved
				Buffer.SetSize( Buffer.GetAllocSize() );
				break;

			//	some socket error, fail
			default:
				return ConnState_DisconnectedBad;
		}
	}
	else
	{
		//	set the buffer size to the amount of data we recieved
		Buffer.SetSize( Result );
	}

	//	push the data into the Peer's buffer
	Peer.OnRecievedPacketPart( Buffer, m_ListeningPeer );

	//	no more data, so notify that we've filled up the data for this Peer
	//	and have a packet to send
	if ( !MoreData )
	{
		OnRecievedPacket( Peer );
	}

	return ConnState_Connected;
}


//----------------------------------------------------------------------------//
//	send queued packets on peer (sending to peer)
//----------------------------------------------------------------------------//
TLNetwork::TConnectionServerSocket::TConnState TLNetwork::TConnectionServerSocket::SendData(TPeerSocket& Peer)
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
SyncBool TLNetwork::TConnectionServerSocket::SendPacket(TPeerSocket& Peer,TPacket& Packet,TLNetwork::TConnectionServerSocket::TConnState& ConnState)
{
	//	check we're trying to send the right packet from the right peer
	if ( Packet.GetToPeer() != Peer )
	{
		if ( !TLDebug_Break("Peer/packet mis-match when sending packet from server") )
			return SyncFalse;
	}

	//	resume packet sending
	const u8* pData = Packet.GetDataToSend();
	u16 DataSize = Packet.GetDataToSendSize();

	//	limit amount to send
	TLMaths::Limit<u16>( DataSize, 0, Packet.GetSendLimit() );
	
	//	send packet
	int Flags = 0;	//	gr: always zero as per winapi
	int Result = send( Peer.GetSocket(), (const char*)pData, DataSize, Flags );

	//	result is number of bytes, or error...
	if ( Result == SOCKET_ERROR )
	{
		int Error = WSAGetLastError();

		switch ( Error )
		{
			//	hardware can't send any more data atm
			case WSAENOBUFS:
				ConnState = ConnState_Connected;
				return SyncWait;

			//	would block... so nothing to do?
			case WSAEWOULDBLOCK:
				ConnState = ConnState_Connected;
				TLDebug_Break("unexpected WouldBlock... was the data sent or not?");
				return SyncWait;

			//	hardware can't send this much data in one go!
			case WSAEMSGSIZE:
				//	note this on the packet and attempt re-send
				ConnState = ConnState_Connected;
				Packet.OnDataTooLarge();
				return SyncWait;
		}

		ConnState = ConnState_DisconnectedBad;
		return SyncFalse;
	}

	//	invalid result
	if ( Result > DataSize || Result < 0 )
	{
		TLDebug_Break("Error!, winsock said we transmitted more bytes than we sent. Assuming okay.");
		Result = DataSize;
	}

	//	increment "how many bytes sent"
	Packet.OnDataSent( (u16)Result );

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
bool TLNetwork::TConnectionServerSocket::SendToPeer(const TBinary& PacketData,TRefRef PeerRef,TRefRef PacketRef)
{
	//	find peer's socket
	TPeerSocket* pPeerSocket = GetPeerSocket( PeerRef );
	if ( !pPeerSocket )
		return false;

	//	queue up on peer
	return pPeerSocket->SendPacket( PacketData, m_ListeningPeer, PacketRef );
}

//---------------------------------------------------------
//	peer connected - notify subscribers
//---------------------------------------------------------
void TLNetwork::TConnectionServerSocket::OnPeerConnected(TPtr<TLNetwork::TPeerSocket>& pPeer)	
{
	if ( !pPeer )
	{
		TLDebug_Break("Peer expected");
		return;
	}

	//	add to list of peers
	s32 PeerIndex = m_Peers.Add( pPeer );

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
void TLNetwork::TConnectionServerSocket::OnPeerDisconnected(TRef PeerRef,bool Gracefully)	
{
	//	find peer
	s32 PeerIndex = m_Peers.FindIndex( PeerRef );
	if ( PeerIndex == -1 )
	{
		TLDebug_Break("Peer expected");
		return;
	}

	//	remove from peer list (should delete unless anything has ptr's still to peer)
	m_Peers.RemoveAt( PeerIndex );

	//	send out notification
	TLMessaging::TMessage Message( TRef_Static(P,R,D,i,s) );
	Message.ExportData( TRef_Static4(P,e,e,r), PeerRef );
	Message.ExportData( TRef_Static(G,r,a,c,e), Gracefully );
	PublishMessage( Message );
}


//---------------------------------------------------------
//	finished recieving a block of data from a client
//---------------------------------------------------------
void TLNetwork::TConnectionServerSocket::OnRecievedPacket(TLNetwork::TPeerSocket& Peer)
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



TLNetwork::TPeerSocket::TPeerSocket(SOCKET& PeerSocket) :
	m_Socket	( PeerSocket )
{
	m_Address = GetConnectedSocketAddress( m_Socket );

	//	failed to resolve address of socket...
	if ( !m_Address.IsValid() )
	{
		//Shutdown();
		return;
	}
}
