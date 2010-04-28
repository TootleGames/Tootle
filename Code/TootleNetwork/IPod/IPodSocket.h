#pragma once

#include "../TSocket.h"

//	cocoa sockets integration
#include <CoreFoundation/CFSocket.h>

//	bsd-sockets include
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

//	SOCKET doesn't exist in cocoa.
typedef int SOCKET;
static const SOCKET	INVALID_SOCKET = -1;
static const int SOCKET_ERROR = -1;



namespace TLNetwork
{
	class TWinsockManager;
	
	namespace platform
	{
		class Socket;
		class PeerSocket;
	}
	
	class TTempAddrInfo;
};


//----------------------------------------------------------------------------//
//	manage an "allocated" addr info, cleans up on destruction
//----------------------------------------------------------------------------//
class TLNetwork::TTempAddrInfo
{
public:
	TTempAddrInfo(TRefRef Protocol,u16 Port);
	~TTempAddrInfo();
	
	bool				IsValid() const		{	return m_pAddrInfo!=NULL;	}
	struct addrinfo&	GetData()			{	return *m_pAddrInfo;	}
	
protected:
	struct addrinfo*	m_pAddrInfo;
};



//----------------------------------------------------------------------------//
//	
//----------------------------------------------------------------------------//
class TLNetwork::Platform::Socket : public TLNetwork::TSocket
{
public:
	Socket();
	virtual ~Socket();
	
	virtual SyncBool	Initialise(u16 Port,TRefRef Protocol);	//	init this listening socket on this port
	
	virtual TAddress	GetAddress() const;						//	get address of socket (ie. our local address). This may be 0.0.0.0:PORT!
	virtual bool		GetNewPeers(TPtrArray<TLNetwork::TPeerSocket>& NewPeerSockets);	//	get any new peer-sockets that have connected to our socket. returns false on winsock error
	
private:
	void				Shutdown();			//	close, shutdown and cleanup the socket
	bool				Listen(const struct addrinfo& AddrInfo,u8 MaxConnections);		//	bind socket to a port and listen for connections
	
protected:
	SOCKET				m_Socket;
	bool				m_IsOpen;			//	if the socket has been opened, we will need to close it!
};


//----------------------------------------------------------------------------//
//	
//----------------------------------------------------------------------------//
class TLNetwork::Platform::PeerSocket : public TLNetwork::TPeerSocket
{
public:
	PeerSocket(SOCKET& PeerSocket);
	~PeerSocket();
	
	virtual TAddress	GetAddress() const;					//	get the address of the peer
	virtual TAddress	GetLocalAddress() const;			//	get our local address which the peer connected to
	
	virtual SyncBool	RecieveData(TArray<u8>& Buffer,TRef& Error);										//	recieve data into buffer. SyncTrue means all data recieved (could be zero!), wait means more data to come, false means error (and disconnection). If we return false and no error is set, then the connection was gracefully closed
	virtual bool		SendData(const u8* pData,u32 DataSize,u32& DataSent,bool& TooLarge,TRef& Error);	//	send data to peer. if false returned, then if the error is invalid then it was a gracefull closure
	
protected:
	SOCKET				m_Socket;							//	socket of Peer
};

