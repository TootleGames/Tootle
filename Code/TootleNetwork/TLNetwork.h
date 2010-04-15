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

	//	gr: maybe these should be hidden away in socket-related classes/namespaces?
	class TAddress;
	class TPeer;
	class TPacket;
}



//----------------------------------------------------------------------------//
//
//----------------------------------------------------------------------------//
class TLNetwork::TAddress
{
public:
	TAddress()								: m_IP ( 0,0,0,0 ), m_Port ( 0 )	{}
	TAddress(const Type4<u8>& IP,u16 Port)	: m_IP ( IP ), m_Port ( Port )	{}
	TAddress(const TAddress& Other)			: m_IP ( Other.m_IP ), m_Port ( Other.m_Port )	{}

	void				GetString(TString& String) const	{	String << m_IP.x << '.' << m_IP.y << '.' << m_IP.z << '.' << m_IP.w << ':' << m_Port;	}
	u16					GetPort() const						{	return m_Port;	}
	const Type4<u8>&	GetIP() const						{	return m_IP;	}

	bool				IsValid() const						{	return (m_IP != Type4<u8>(0,0,0,0)) || (m_Port!=0);	}	//	local host often appears as 0,0,0,0. so an invalid address must be all-zeros (including port)
	void				SetInvalid()						{	m_IP.Set(0,0,0,0);	m_Port = 0;	}

protected:
	Type4<u8>				m_IP;
	u16						m_Port;
};


//--------------------------------------------------------
//	append address description
//--------------------------------------------------------
template<>
FORCEINLINE TString& operator<<(TString& String,const TLNetwork::TAddress& Value)
{
	Value.GetString( String );
	return String;
}

//----------------------------------------------------------------------------//
//	class identifying a peer (client, server etc). *could* expose this (const) outside of networking lib if required
//----------------------------------------------------------------------------//
class TLNetwork::TPeer
{
public:
	TPeer();

	TRefRef				GetPeerRef() const						{	return m_PeerRef;	}
	void				GetDescription(TString& String) const;	//	append description of this Peer to the string

	FORCEINLINE bool	operator==(TRefRef PeerRef) const		{	return GetPeerRef() == PeerRef;	}
	FORCEINLINE bool	operator!=(TRefRef PeerRef) const		{	return GetPeerRef() != PeerRef;	}
	FORCEINLINE bool	operator==(const TPeer& Peer) const		{	return GetPeerRef() == Peer.GetPeerRef();	}
	FORCEINLINE bool	operator!=(const TPeer& Peer) const		{	return GetPeerRef() != Peer.GetPeerRef();	}

private:
	TRef				m_PeerRef;								//	unique Peer ref
};


//----------------------------------------------------------------------------//
//	gr: this is a packet for the network system to deal with... probably don't want to expose this
//		outside the network system
//----------------------------------------------------------------------------//
class TLNetwork::TPacket
{
public:
	TPacket(TRefRef PacketRef,const TBinary& Data,const TPeer& ToPeer,const TPeer& FromPeer);

	TRefRef			GetPacketRef() const			{	return m_PacketRef;	}

	TBinary&		GetData()						{	return m_Data;	}
	const TBinary&	GetData() const					{	return m_Data;	}
	u16				GetDataSize()					{	return m_Data.GetSize();	}
	const u8*		GetDataToSend()					{	return m_Data.GetData( m_DataSent );	}
	u16				GetDataToSendSize() const		{	return GetData().GetSize() - m_DataSent;	}
	u16				GetDataSentSize() const			{	return m_DataSent;	}

	void			OnDataSent(u16 BytesSent);		//	increment amount of data sent.
	void			OnDataTooLarge()				{	m_SendSmall = true;	}	//	if the hardware said we've tried to send too much data in one go, then explicitly send a small amount
	u16				GetSendLimit() const			{	return m_SendSmall ? 600 : 0xffff;	}	//	limit amount of data to send in one go

	const TPeer&	GetFromPeer() const				{	return *m_pFromPeer;	}
	const TPeer&	GetToPeer() const				{	return *m_pToPeer;	}

protected:
	const TPeer*	m_pFromPeer;	//	who sent it
	const TPeer*	m_pToPeer;		//	who's recieving it
	TRef			m_PacketRef;	//	packet ref. may not be valid for incoming packets, but for outgoing packets this is a way to identify particular packets. In future our own protcol will set & extract packet refs, and can be used for auto ACK's
	TBinary			m_Data;			//	data!
	u16				m_DataSent;		//	amount of the data we have sent. only used when sending packets
	bool			m_SendSmall;	//	send only a small amount of data at a time
};

