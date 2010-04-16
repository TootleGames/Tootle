/*------------------------------------------------------
	

-------------------------------------------------------*/
#pragma once

#include "TSocket.h"
#include <TootleCore/TRef.h>
#include <TootleCore/TPtr.h>
#include <TootleCore/TBinaryTree.h>
#include <TootleCore/TString.h>
#include <TootleCore/TRelay.h>



namespace TLNetwork
{
	class TSocketServer;
	class TClient;		
};


//----------------------------------------------------------------------------//
//	gr: not exposed outside network code
//----------------------------------------------------------------------------//
class TLNetwork::TClient : public TLNetwork::TPeer
{
public:
	TClient(TPtr<TPeerSocket>& pPeerSocket);

	TPeerSocket&		GetSocket()									{	return *m_pSocket;	}
	const TPeerSocket&	GetSocket() const							{	return *m_pSocket;	}
	void				GetDescription(TString& String) const;		//	append description of this Peer to the string
	TAddress			GetPeerAddress() const						{	return m_PeerAddress;	}
	TAddress			GetLocalAddress() const						{	return GetSocket().GetLocalAddress();	}

	TPtr<TPacket>&		GetRecievingPacket()						{	return m_pRecievingPacket;	}
	void				OnRecievedPacketPart(TArray<u8>& Data,const TPeer& ServerPeer);		//	recieved some packet data - append it to our packet
	void				FlushRecievingPacket()						{	m_pRecievingPacket = NULL;	}

	bool				SendPacket(const TBinary& Data,const TPeer& FromPeer,TRefRef PacketRef)	{	TPtr<TPacket> pPacket = new TPacket( PacketRef, Data, *this, FromPeer );	return m_SendPackets.Add( pPacket ) != -1;	}	//	queue a packet for sending
	TPtrArray<TPacket>&	GetSendPacketQueue()						{	return m_SendPackets;	}

protected:
	TAddress				m_PeerAddress;			//	cached address of socket
	TPtr<TPeerSocket>		m_pSocket;				//	socket
	TPtr<TPacket>			m_pRecievingPacket;		//	current packet we're recieving
	TPtrArray<TPacket>		m_SendPackets;			//	queue of packets to send
};


//--------------------------------------------------------
//	append Peer description
//--------------------------------------------------------
template<>
FORCEINLINE TString& operator<<(TString& String,const TLNetwork::TClient& Value)
{
	Value.GetDescription( String );
	return String;
}


//--------------------------------------------
//	socket manager
//--------------------------------------------
class TLNetwork::TSocketServer : public TLMessaging::TPublisherSubscriber
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
	TSocketServer(TBinaryTree& ServerInfo);
	~TSocketServer();

	SyncBool		Initialise(TRef& ErrorRef);				//	initialise. if failed, ErrorRef will be set to an error
	bool			Update();								//	regular update. return false if error with winsock and need to shutdown

	virtual TRefRef	GetSubscriberRef() const				{	static TRef Ref="SrvSckt";	return Ref;	}
	TAddress		GetLocalAddress()						{	return m_pListeningSocket ? m_pListeningSocket->GetAddress() : TAddress();	}	//	get local address
	TAddress		GetLocalAddress(TRefRef Peer)			{	TClient* pClient = GetClient( Peer );	return pClient ? pClient->GetLocalAddress() : TAddress();	}	//	get our local address according to this peer

	bool			SendToPeer(const TBinary& PacketData,TRefRef PeerRef,TRefRef PacketRef=TRef());		//	send this packet to this peer

protected:
	virtual void	ProcessMessage(TLMessaging::TMessage& Message);

private:
	bool		CheckForNewConnections();						//	check for new connections. returns false if there's an error with winsock and we need to shutdown
	TConnState	RecieveData(TClient& Peer);						//	recieve new data from client. 
	TConnState	SendData(TClient& Peer);						//	send queued packets on peer (sending to peer)
	SyncBool	SendPacket(TClient& Peer,TPacket& Packet,TConnState& State);	//	send this packet to the peer. returns true if sent, false if failed, wait if only partially sent

	void		OnPeerConnected(TPtr<TClient>& pPeer);					//	peer connected - notify subscribers
	void		OnPeerDisconnected(TRef PeerRef,bool Gracefully);		//	peer disconnected
	void		OnPeerDisconnected(const TPeer& Peer,bool Gracefully)	{	return OnPeerDisconnected( Peer.GetPeerRef(), Gracefully );	}
	void		OnRecievedPacket(TClient& Peer);						//	finished recieving a packet from a peer - send it out to subscribers

	TClient*	GetClient(TRefRef PeerRef)								{	return m_Clients.FindPtr( PeerRef );	}

protected:
	TPeer						m_ListeningPeer;		//	listening socket peer info
	TPtr<TSocket>				m_pListeningSocket;		//	our listening socket
	TPtrArray<TClient>			m_Clients;				//	array of connected clients
	u16							m_Port;					//	what port are we listening on
	TRef						m_Protocol;				//	TCP or UDF protocol
};
