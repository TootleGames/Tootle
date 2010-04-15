/*------------------------------------------------------
	
	PC implementation of sockets

	gr: some of this may move to be generic vs platform socket stuff
		some server vs client stuff might be abstracted too

-------------------------------------------------------*/
#pragma once

#pragma comment( lib, "ws2_32.lib" )

#include "../TLNetwork.h"
#include <TootleCore/TRef.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TBinaryTree.h>
#include <TootleCore/TString.h>
#include <TootleCore/TRelay.h>


//	include winsock headers
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>


namespace TLNetwork
{
	class TWinsockManager;
	class TConnectionServerSocket;
	class TSocket;
	class TPeerSocket;
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
//	single socket management. cleans up socket on destruction - private to network lib
//----------------------------------------------------------------------------//
class TLNetwork::TSocket
{
public:
	TSocket(const struct addrinfo& AddrInfo);
	~TSocket();

	bool			IsValid() const		{	return m_Socket != INVALID_SOCKET;	}
	SOCKET&			GetData()			{	return m_Socket;	}

	const TAddress&	GetAddress() const	{	return m_Address;	}

	bool			Bind(const struct addrinfo& AddrInfo);		//	bind socket to this address

private:
	void			Shutdown();			//	close, shutdown and cleanup the socket

protected:
	SOCKET		m_Socket;
	bool		m_IsOpen;			//	if the socket has been opened, we will need to close it!
	TAddress	m_Address;			//	cache of address of socket
};



//----------------------------------------------------------------------------//
//	gr: not exposed outside network code
//----------------------------------------------------------------------------//
class TLNetwork::TPeerSocket : public TLNetwork::TPeer
{
public:
	TPeerSocket(SOCKET& PeerSocket);
	~TPeerSocket();

	SOCKET&				GetSocket()				{	return m_Socket;	}

	TPtr<TPacket>&		GetRecievingPacket()						{	return m_pRecievingPacket;	}
	void				OnRecievedPacketPart(TArray<u8>& Data,const TPeer& ServerPeer);		//	recieved some packet data - append it to our packet
	void				FlushRecievingPacket()						{	m_pRecievingPacket = NULL;	}

	bool				SendPacket(const TBinary& Data,const TPeer& FromPeer,TRefRef PacketRef)	{	TPtr<TPacket> pPacket = new TPacket( PacketRef, Data, *this, FromPeer );	return m_SendPackets.Add( pPacket ) != -1;	}	//	queue a packet for sending
	TPtrArray<TPacket>&	GetSendPacketQueue()						{	return m_SendPackets;	}

	//	inherited?
//	FORCEINLINE bool	operator==(TRefRef PeerRef) const	{	return GetPeerRef() == PeerRef;	}

protected:
	TAddress				m_Address;				//	address of the peer 
	SOCKET					m_Socket;				//	socket of Peer
	TPtr<TPacket>			m_pRecievingPacket;		//	current packet we're recieving
	TPtrArray<TPacket>		m_SendPackets;			//	queue of packets to send
};


//--------------------------------------------------------
//	append Peer description
//--------------------------------------------------------
template<>
FORCEINLINE TString& operator<<(TString& String,const TLNetwork::TPeerSocket& Value)
{
	Value.GetDescription( String );
	return String;
}


//----------------------------------------------------------------------------//
//	internal (PC specific) winsock initialsation/shutdown manager
//----------------------------------------------------------------------------//
class TLNetwork::TWinsockManager
{
public:
	TWinsockManager();
	~TWinsockManager();

	bool		IsInitialised() const	{	return m_DataValid;	}

protected:
	bool		m_DataValid;	//	has the winsock data been initialised?
	WSADATA		m_Data;			//	winsock data
};

//--------------------------------------------
//	socket manager
//--------------------------------------------
class TLNetwork::TConnectionServerSocket : public TLMessaging::TPublisherSubscriber
{
private:
	//	result from socket operations to get current state of the socket after processing
	enum TConnState
	{
		ConnState_Invalid=0,		//	invalid/uninitialised state
		ConnState_Connected,		//	still connected
		ConnState_DisconnectedGood,	//	disconnected gracefully
		ConnState_DisconnectedBad,	//	error/sudden disconnection (including an unexpected "is already disconnected")
	};

public:
	TConnectionServerSocket(TBinaryTree& ServerInfo);
	~TConnectionServerSocket();

	virtual TRefRef	GetSubscriberRef() const			{	static TRef Ref="SrvSckt";	return Ref;	}

	SyncBool	Initialise(TRef& ErrorRef);				//	initialise. if failed, ErrorRef will be set to an error
	bool		Update();								//	regular update. return false if error with winsock and need to shutdown

	TAddress	GetAddress()							{	return m_pListeningSocket ? m_pListeningSocket->GetAddress() : TAddress();	}

	bool		SendToPeer(const TBinary& PacketData,TRefRef PeerRef,TRefRef PacketRef=TRef());		//	send this packet to this peer

protected:
	virtual void	ProcessMessage(TLMessaging::TMessage& Message);

private:
	bool		CheckForNewConnections();						//	check for new connections. returns false if there's an error with winsock and we need to shutdown
	TConnState	RecieveData(TPeerSocket& Peer);					//	recieve new data from client. 
	TConnState	SendData(TPeerSocket& Peer);					//	send queued packets on peer (sending to peer)
	SyncBool	SendPacket(TPeerSocket& Peer,TPacket& Packet,TConnState& State);	//	send this packet to the peer. returns true if sent, false if failed, wait if only partially sent

	void		OnPeerConnected(TPtr<TPeerSocket>& pPeer);				//	peer connected - notify subscribers
	void		OnPeerDisconnected(TRef PeerRef,bool Gracefully);		//	peer disconnected
	void		OnPeerDisconnected(const TPeer& Peer,bool Gracefully)	{	return OnPeerDisconnected( Peer.GetPeerRef(), Gracefully );	}
	void		OnRecievedPacket(TPeerSocket& Peer);	//	finished recieving a packet from a peer - send it out to subscribers

	TPeerSocket*				GetPeerSocket(TRefRef PeerRef)	{	return m_Peers.FindPtr( PeerRef );	}

protected:
	TPtr<TWinsockManager>		m_pWinsock;				//	winsock manager
	TPeer						m_ListeningPeer;		//	listening socket peer info
	TPtr<TSocket>				m_pListeningSocket;		//	our listening socket
	TPtrArray<TPeerSocket>		m_Peers;				//	array of connected clients
	u16							m_Port;					//	what port are we listening on
	TRef						m_Protocol;				//	TCP or UDF protocol
};
