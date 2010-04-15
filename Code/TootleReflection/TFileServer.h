/*------------------------------------------------------

	The reflection file server is an FTP server that exposes
	all our file systems at runtime to any FTP client that wants to
	connect to the game.

	Always using port 7007 (that's supposed to be TOOT-le btw)

-------------------------------------------------------*/
#pragma once

#include <TootleCore/TLTypes.h>
#include <TootleCore/TRelay.h>
#include <TootleCore/TKeyArray.h>
#include <TootleFileSys/TLFileSys.h>
#include <TootleNetwork/TLNetwork.h>
#include <TootleNetwork/PC/PCConnectionServerSocket.h>



namespace TLReflection
{
	static const u16	g_Port = 21;	//	connection port
	SyncBool	StartFileServer();		//	create file server
	SyncBool	StopFileServer();		//	kill file server

	class TFileServer;
	class TFtpClient;
	class TFtpCommand;
	class TFtpDataTask;

	class TFtpDataTaskDirListing;
	class TFtpDataTaskSendFile;
	class TFtpDataTaskRecieveFile;
};


//----------------------------------------------------------------------------//
//	ftp command recieved from a client
//----------------------------------------------------------------------------//
class TLReflection::TFtpCommand
{
public:
	TFtpCommand(TRefRef CommandRef,const TString& CommandArgs) :
		m_Ref		( CommandRef ),
		m_Arguments	( CommandArgs )
	{
	}

public:
	TRef		m_Ref;			//	ftp command, STOR, DELE, CWD etc
	TString		m_Arguments;	//	args for the command
};


//----------------------------------------------------------------------------
//	base data task
//----------------------------------------------------------------------------
class TLReflection::TFtpDataTask
{
public:
	TFtpDataTask();
	virtual ~TFtpDataTask()	{}

	virtual SyncBool	GetState();								//	get state of the task - SyncTrue finished(or cancelled), SyncWait.. still going, SyncFalse... failed

	virtual void		OnPacketRecieved(TBinary& Data)			{}	//	recieved packet on data connect
	virtual void		OnPacketSent(TRefRef PacketRef)			{	if ( PacketRef == TRef_Static4(D,a,t,a) )	m_PacketSent = SyncTrue;	}
	virtual void		OnPeerConnected(TPtr<TLNetwork::TConnectionServerSocket>& pSocket,TRefRef Peer);	//	our client has connected on the data connection, send out the queued packet (if there is one). returns false if there is a problem and we need to reject the connection (eg. peer is already connected)
	virtual void		OnPeerDisconnected(bool Gracefully)		{}	//	peer has been disconnected (either on purpose or by error/abortion)
	void				OnError()								{	m_PacketSent = SyncFalse;	}	//	non-specific fail-this-task call

protected:
	void				SendPacket(TBinary& PacketData);		//	queue up a packet to be sent when the peer connects

private:
	TPtr<TBinary>	m_pSendPacketData;							//	packet to send to peer when they've connected
	SyncBool		m_PacketSent;								//	state of the queued packet (will remain SyncWait if we have no packet to send)
	TRef			m_PeerRef;									//	peer to send data to, set once they have connected
	TPtr<TLNetwork::TConnectionServerSocket>	m_pSocket;		//	socket to send data on
};


//----------------------------------------------------------------------------
//	task to send directory listing to peer
//----------------------------------------------------------------------------
class TLReflection::TFtpDataTaskDirListing : public TLReflection::TFtpDataTask
{
public:
	TFtpDataTaskDirListing(const TString& DirListing);
};


//----------------------------------------------------------------------------
//	task to send file to peer
//----------------------------------------------------------------------------
class TLReflection::TFtpDataTaskSendFile : public TLReflection::TFtpDataTask
{
public:
	TFtpDataTaskSendFile(TPtr<TLFileSys::TFile>& pFile);	//	gr: not const in case we need to load the file from the file sys and update it
};



//----------------------------------------------------------------------------
//	task to send file to peer
//----------------------------------------------------------------------------
class TLReflection::TFtpDataTaskRecieveFile : public TLReflection::TFtpDataTask
{
public:
	TFtpDataTaskRecieveFile(TPtr<TLFileSys::TFile>& pNewFile);

	virtual SyncBool		GetState();			//	get state of the task - SyncTrue finished(or cancelled), SyncWait.. still going, SyncFalse... failed

protected:
	virtual void			OnPacketRecieved(TBinary& Data);		//	recieved packet from data server
	virtual void			OnPeerDisconnected(bool Gracefully);	//	peer has been disconnected (either on purpose or by error/abortion)

protected:
	TPtr<TLFileSys::TFile>	m_pRecievingFile;
};




//----------------------------------------------------------------------------//
//	ftp client - only accessible by the FtpServer (TFileServer)
//----------------------------------------------------------------------------//
class TLReflection::TFtpClient : public TLMessaging::TSubscriber
{
protected:
	enum DataMode
	{
		DataMode_Idle=0,		//	data server not doing anything
		DataMode_DirListing,	//	sending directory listing
		DataMode_SendingFile,	//	sending file to client
		DataMode_RecievingFile,	//	recieving file from client
	};
public:
	TFtpClient(TRefRef PeerRef);
	virtual ~TFtpClient()	{}

	void		Update(TLNetwork::TConnectionServerSocket& Server);				//	update routine, required to catch the finishing of a data transfer

	TRefRef		GetPeerRef() const					{	return m_PeerRef;	}

	bool		SendPacket(u32 FtpCommand,const TString& String,TLNetwork::TConnectionServerSocket& Server);

	void		AddPendingCommand(TRefRef CommandRef,const TString& CommandArgs)	{	m_CommandQueue.AddNewPtr( new TFtpCommand(CommandRef, CommandArgs) );	}	//	add command to client to process
	bool		ExecuteCommands(TLNetwork::TConnectionServerSocket& Server);		//	execute any/all the commands we've put onto the client. return false if we want to disconnect the client

protected:
	virtual TRefRef		GetSubscriberRef() const				{	static TRef Ref("FtpTask");	return Ref;	}
	virtual void		ProcessMessage(TLMessaging::TMessage& Message);

	bool		ExecuteCommand(const TFtpCommand& Command,TLNetwork::TConnectionServerSocket& Server);		//	execute a command, return false if we want to close the connection
	void		OnDataTaskFinished(bool Success,TLNetwork::TConnectionServerSocket& CommandServer);	//	current data task has finished

	//	some common commands to send
	bool		SendPacketCurrentDir(u32 FtpCommand,TLNetwork::TConnectionServerSocket& Server);
	bool		GetDirectoryList(const TString& Arguments,TString& DirListing,bool JustNames) const;

	bool		CreateDataServer();										//	create data server (this gets pre-created for PASV mode)
	void		DestroyDataServer();									//	destroy, close and clean up the data server
	bool		CreateDataTask(TPtr<TFtpDataTask>& pNewTask,TLNetwork::TConnectionServerSocket& CommandServer);			//	create a data server to transfer something
	void		DestroyDataTask();										//	clean up the task
	bool		SendDataDirListing(const TString& DirListing,TLNetwork::TConnectionServerSocket& CommandServer);			//	send dir listing via data connection
	bool		SendDataSendFile(TPtr<TLFileSys::TFile>& pFile,TLNetwork::TConnectionServerSocket& CommandServer);	//	send file via data connection
	bool		SendDataRecieveFile(TPtr<TLFileSys::TFile>& pNewFile,TLNetwork::TConnectionServerSocket& CommandServer);	//	recieve into this file via data connection

protected:
	TRef					m_FileSystem;		//	where in the file system the client is. if invalid then currently browsing the root. as each "dir" is a file system, we can simply represent their CWD this way
	TPtrArray<TFtpCommand>	m_CommandQueue;		//	commands from the client we've yet to process

	TPtr<TLNetwork::TConnectionServerSocket>	m_pDataServer;		//	data server socket for this client - gr: this could be stored on the task, (for multiple transfers at once) but I've stored it here to re-use it
	TPtr<TFtpDataTask>		m_pDataTask;		//	data-task to send some data on the data port
	TRef					m_DataPeer;			//	peer ref connected to the data socket

	bool					m_PassiveMode;		//	in passive mode

private:
	TRef		m_PeerRef;		//	ref of the peer
	TString		m_Username;		
};


class TLReflection::TFileServer : public TLMessaging::TPublisherSubscriber
{
public:
	TFileServer();
	virtual ~TFileServer();

	SyncBool			Initialise();
	void				Update();											//	update clients

protected:
	virtual TRefRef		GetSubscriberRef() const							{	static TRef Ref = TRef_Static(F,S,r,v,r);	return Ref;	}
	virtual void		ProcessMessage(TLMessaging::TMessage& Message);

	void				OnPeerConnected(TRefRef PeerRef);					//	new peer connected - setup ftp client
	void				OnPeerDisconnected(TRefRef PeerRef);				//	peer disconnected
	void				OnPacket(TBinary& Packet,TFtpClient& Client);		//	this client sent a new ftp packet

protected:
	TPtr<TLNetwork::TConnectionServerSocket>	m_pServer;					//	command server socket
	TPtrKeyArray<TRef,TFtpClient>				m_FtpClients;				//	ftp client for each peer
};

