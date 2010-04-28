#include "MacSocket.h"
#include <TootleCore/TLCore.h>
#include <TootleCore/TCoreManager.h>



//----------------------------------------------------------
//	this is the unix equivelent of WSAGetLastError
//----------------------------------------------------------
int WSAGetLastError()
{
	//	in the unix kernal (osx) the last-error is set to the errno global
	extern int errno;
	return errno;
}


TLNetwork::TAddress GetSockAddrAddress(const sockaddr_storage& SocketAddress,bool NetworkAddress)
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
	if ( NetworkAddress )
	{
		SocketAddress4.sin_port = ntohs( SocketAddress4.sin_port );
		SocketAddress4.sin_addr.s_addr = ntohl( SocketAddress4.sin_addr.s_addr );
	}
	
	//	set address
	u8* pAddr = (u8*)SocketAddress4.sin_addr.s_addr;
	TLNetwork::TAddress Result( Type4<u8>( pAddr[0], pAddr[1], pAddr[2], pAddr[3] ), SocketAddress4.sin_port );
	
	TDebugString Debug_String;
	Debug_String << "Resolved socket to " << Result;
	TLDebug_Print( Debug_String );
	
	return Result;
}




TLNetwork::Platform::Socket::Socket() :
	m_Socket	( INVALID_SOCKET ),
	m_IsOpen	( false )
{
}


TLNetwork::Platform::Socket::~Socket()
{
	Shutdown();
}


//-----------------------------------------------------------------
//	init this listening socket on this port
//-----------------------------------------------------------------
SyncBool TLNetwork::Platform::Socket::Initialise(u16 Port,TRefRef Protocol)
{
    //	Resolve the local address and port
	TTempAddrInfo ServerAddrInfo( Protocol, Port );
	if ( !ServerAddrInfo.IsValid() )
        return SyncFalse;
	
	const struct addrinfo& AddrInfo = ServerAddrInfo.GetData();
	
	//	create socket
	m_Socket = socket( AddrInfo.ai_family, AddrInfo.ai_socktype, AddrInfo.ai_protocol );
	if ( m_Socket == INVALID_SOCKET )
		return SyncFalse;
	
	//	make socket non-blocking
	int Result = fcntl( m_Socket, F_SETFL, O_NONBLOCK );
	if ( Result == -1 )
	{
		int Error = WSAGetLastError();
		Shutdown();
		return SyncFalse;
	}
	
	//	bind and listen the socket
	if ( !Listen( AddrInfo, (u8)SOMAXCONN ) )
	{
		Shutdown();
		return SyncFalse;
	}
	
	return SyncTrue;
}


//-----------------------------------------------------------------
//	close socket
//-----------------------------------------------------------------
void TLNetwork::Platform::Socket::Shutdown()
{
	//	delete socket
	if ( m_Socket != INVALID_SOCKET )
	{
//xxx		close( m_Socket );
		m_Socket = INVALID_SOCKET;
		m_IsOpen = false;
	}
}


//-----------------------------------------------------------------
//	resolve address of this socket
//	gr: note; this may give an address of 0.0.0.0 (IP_ANY, but with a correct port)
//		unless the socket was opened on a particular address. To get a socket's address
//		of which the peer connected to (eg. peer connected to 127.0.0.1, or 192.168.0.1)
//		then you need to use GetLocalAddress on TPeerSocket
//-----------------------------------------------------------------
TLNetwork::TAddress TLNetwork::Platform::Socket::GetAddress() const
{
	//	get address
	sockaddr_storage SocketAddress;
	socklen_t SizeOfSocketAddress = sizeof(SocketAddress);
	
	//	get peer's name from their socket (a socket we're connected to)
	int Result = getsockname( m_Socket, (sockaddr*)&SocketAddress, &SizeOfSocketAddress );
	if ( Result == SOCKET_ERROR )
	{
		//	check the error
		int Error = WSAGetLastError();
		return TAddress();
	}
	
	return GetSockAddrAddress( SocketAddress, true );
}


//-----------------------------------------------------------------
//	bind socket to this port and protocol
//-----------------------------------------------------------------
bool TLNetwork::Platform::Socket::Listen(const struct addrinfo& AddrInfo,u8 MaxConnections)
{
	if ( m_Socket == INVALID_SOCKET )
	{
		TLDebug_Break("valid socket expected");
		return false;
	}
	
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
		
		//	if we get EACCESS on a unix system, it's probbaly because only root is allowed
		//	to bind to a port < 1024 as these are priviliged. Try a different port.
		if ( Error == EACCES )
			return false;
		
		return false;
	}
	
	//	will need closing now
	m_IsOpen = true;
	
	//	allow sockets to connect
	Result = listen( m_Socket, MaxConnections );
	if ( Result == SOCKET_ERROR )
	{
		//	check the error
		int Error = WSAGetLastError();
		return false;
	}
	
	return true;
}




//-----------------------------------------------------------------
//	get any new peer-sockets that have connected to our socket. returns false on winsock error
//-----------------------------------------------------------------
bool TLNetwork::Platform::Socket::GetNewPeers(TPtrArray<TLNetwork::TPeerSocket>& NewPeerSockets)
{
	// Accept a Peer socket
	SOCKET PeerSocketHandle = accept( m_Socket, NULL, NULL );
	
	//	no incoming connection
	if ( PeerSocketHandle == INVALID_SOCKET )
	{
		//	check the error
		int Error = WSAGetLastError();
		
		//	if would block, then that means if it was a blocking socket we'd stick here until
		//	someone did try and connect. so to us we've just not had a connection request 
		if ( Error == EWOULDBLOCK )
			return true;
		
		//	some error! kill winsock!
		return false;
	}
	
	//	add new socket
	TPtr<TPeerSocket> pPeerSocket = new Platform::PeerSocket( PeerSocketHandle );
	NewPeerSockets.Add( pPeerSocket );
	
	return true;
}


TLNetwork::TTempAddrInfo::TTempAddrInfo(TRefRef Protocol,u16 Port) :
m_pAddrInfo	( NULL )
{
	//	setup the addrinfo we want
	struct addrinfo hints;
	
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



TLNetwork::Platform::PeerSocket::PeerSocket(SOCKET& Socket) :
m_Socket	( Socket )
{
}



TLNetwork::Platform::PeerSocket::~PeerSocket()
{
    // cleanup
	if ( m_Socket != INVALID_SOCKET )
	{
//xxx		close( m_Socket );
		m_Socket = INVALID_SOCKET;
	}
}


//---------------------------------------------------------
//	get the address of the peer
//---------------------------------------------------------
TLNetwork::TAddress TLNetwork::Platform::PeerSocket::GetAddress() const
{
	//	get address
	sockaddr_storage SocketAddress;
	socklen_t SizeOfSocketAddress = sizeof(SocketAddress);
	
	//	get peer's name from their socket (a socket we're connected to)
	int Result = getpeername( m_Socket, (sockaddr*)&SocketAddress, &SizeOfSocketAddress );
	if ( Result == SOCKET_ERROR )
	{
		//	check the error
		int Error = WSAGetLastError();
		return TAddress();
	}
	
	return GetSockAddrAddress( SocketAddress, false );
}



//---------------------------------------------------------
//	get our local address which the peer connected to
//---------------------------------------------------------
TLNetwork::TAddress TLNetwork::Platform::PeerSocket::GetLocalAddress() const
{
	//	get address
	sockaddr_storage SocketAddress;
	socklen_t SizeOfSocketAddress = sizeof(SocketAddress);
	
	//	get peer's name from their socket (a socket we're connected to)
	int Result = getsockname( m_Socket, (sockaddr*)&SocketAddress, &SizeOfSocketAddress );
	if ( Result == SOCKET_ERROR )
	{
		//	check the error
		int Error = WSAGetLastError();
		return TAddress();
	}
	
	return GetSockAddrAddress( SocketAddress, false );
}

//---------------------------------------------------------
//	recieve data into buffer. SyncTrue means all data recieved (could be zero!), wait means more data to come, false means error (and disconnection). If we return false and no error is set, then the connection was gracefully closed
//---------------------------------------------------------
SyncBool TLNetwork::Platform::PeerSocket::RecieveData(TArray<u8>& Buffer,TRef& Error)
{
	//	init results
	Error.SetInvalid();
	
	//	gr: note; for udp, we lose excess data so we need to peek data!
	
	//	alloc some data in the buffer to write into
	const u32 PacketBufferSize = 2048;
	Buffer.AddAllocSize( PacketBufferSize );
	s32 BufferStart = Buffer.GetSize();
	s32 BufferSize = Buffer.GetAllocSize() - BufferStart;	//	we presume this will be PacketBufferSize unless there are limits imposed by the array
	if ( BufferSize <= 0 )
	{
		TLDebug_Break("Array buffer-allocation error for packet");
		return SyncWait;
	}
	
	//	get address where we're going to write data into
	u8* pBufferData = &Buffer[BufferStart];
	
	//	recieve data
	int RecieveFlags = 0x0;
	int Result = recv( m_Socket, (char*)pBufferData, BufferSize, RecieveFlags );
	
	//	connection was gracefully closed
	if ( Result == 0 )
	{
		Error.SetInvalid();
		return SyncFalse;
	}
	
	//	this is true if we recieved data up to the limit of our buffer and we need to get some more
	bool MoreData = false;
	
	//	check for error
	if ( Result == SOCKET_ERROR )
	{
		int LastError = WSAGetLastError();
		
		switch ( LastError )
		{
				//	socket closed (nicely?) by peer
			case ECONNRESET:
				Error.SetInvalid();	//	gracefull error
				return SyncFalse;
				
				//	not an error, just nothing to receive
			//case WSAEWOULDBLOCK:
			case EAGAIN:
				return SyncTrue;
		/*
				//	buffer was filled, but there's more to come
			case WSAEMSGSIZE:
				MoreData = true;
				//	re-size the array to it's new size
				Buffer.SetSize( BufferStart + BufferSize );
				break;
		*/
				//	some socket error, fail
			default:
				Error = "SomeErr";
				return SyncFalse;
		}
	}
	else
	{
		//	set the buffer size to the amount of data we recieved
		Buffer.SetSize( BufferStart + Result );
	}
	
	//	if there is more data to come, return sync wait
	if ( MoreData )
		return SyncWait;
	
	//	packet recieved
	return SyncTrue;
}


//---------------------------------------------------------
//	send data to peer. if false returned, then if the error is invalid then it was a gracefull closure
//---------------------------------------------------------
bool TLNetwork::Platform::PeerSocket::SendData(const u8* pData,u32 DataSize,u32& DataSent,bool& TooLarge,TRef& Error)
{
	//	init results
	Error.SetInvalid();
	DataSent = 0;
	TooLarge = false;
	
	//	send packet
	int Flags = 0;	//	gr: always zero as per winapi
	int Result = send( m_Socket, (const char*)pData, DataSize, Flags );
	
	//	result is number of bytes, or error...
	if ( Result == SOCKET_ERROR )
	{
		int SockError = WSAGetLastError();
		
		switch ( SockError )
		{
				//	hardware can't send any more data atm
			case EAGAIN:
			case ENOBUFS:
				DataSent = 0;
				return true;
				
				//	hardware can't send this much data in one go!
			case EMSGSIZE:
				//	note this on the packet and attempt re-send
				TooLarge = true;
				DataSent = 0;
				return true;
		}
		
		Error = "Error";
		return false;
	}
	
	//	invalid result
	if ( Result > (int)DataSize || Result < 0 )
	{
		TLDebug_Break("Error!, winsock said we transmitted more bytes than we sent. Assuming okay.");
		Result = DataSize;
	}
	
	//	record number of bytes sent
	DataSent = Result;
	
	return true;
}


