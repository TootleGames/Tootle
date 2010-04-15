/*------------------------------------------------------


	ftp protocol:
		http://www.faqs.org/rfcs/rfc959.html

-------------------------------------------------------*/
#pragma once

#include "TFileServer.h"
#include <TootleFileSys/TLFileSys.h>
#include <TootleCore/TLCore.h>
#include <TootleCore/TCoreManager.h>


namespace TLReflection
{
	TPtr<TFileServer>	g_pFileServer;
}



//-------------------------------------------------------
//	create file server
//-------------------------------------------------------
SyncBool TLReflection::StartFileServer()
{
	//	already got a file server, make sure it's initialised
	if ( g_pFileServer )
	{
		SyncBool InitResult = g_pFileServer->Initialise();

		//	delete server if we failed
		if ( InitResult == SyncFalse )
			g_pFileServer = NULL;

		return InitResult;
	}

	//	create new file server
	g_pFileServer = new TFileServer;

	//	and init it
	return StartFileServer();
}


//-------------------------------------------------------
//	kill file server
//-------------------------------------------------------
SyncBool TLReflection::StopFileServer()
{
	g_pFileServer = NULL;
	return SyncTrue;
}


//-------------------------------------------------------
//	
//-------------------------------------------------------
TLReflection::TFileServer::TFileServer()
{
}


//-------------------------------------------------------
//	
//-------------------------------------------------------
TLReflection::TFileServer::~TFileServer()
{
}



//-------------------------------------------------------
//	
//-------------------------------------------------------
SyncBool TLReflection::TFileServer::Initialise()
{
	//	create server socket
	if ( !m_pServer )
	{
		//m_pServer = TLNetwork::CreateConnection("SrvSock");
		TBinaryTree ServerInfo( TRef("Command") );
		ServerInfo.ExportData("Protocol", TRef("TCP") );
		ServerInfo.ExportData("Port", (u16)g_Port );
		m_pServer = new TLNetwork::TConnectionServerSocket( ServerInfo );

		//	failed to alloc server
		if ( !m_pServer )
			return SyncFalse;
	}

	//	initialise
	TRef Error;
	SyncBool Result = m_pServer->Initialise(Error);
	if ( Result == SyncFalse )
	{
		m_pServer = NULL;
		return SyncFalse;
	}

	//	success, get messages from server
	if ( Result == SyncTrue )
	{
		this->SubscribeTo( m_pServer );

		//	get updates
		this->SubscribeTo( TLCore::g_pCoreManager );
	}

	//	either still initialising or success...
	return Result;
}


//-------------------------------------------------------
//	
//-------------------------------------------------------
void TLReflection::TFileServer::Update()
{
	//	update clients
	if ( m_pServer )
	{
		for ( u32 c=0;	c<m_FtpClients.GetSize();	c++ )
		{
			TFtpClient& Client = *m_FtpClients.GetItemAt(c);
			Client.Update( *m_pServer );
		}
	}
		

}


//-------------------------------------------------------
//	
//-------------------------------------------------------
void TLReflection::TFileServer::ProcessMessage(TLMessaging::TMessage& Message)
{
	switch ( Message.GetMessageRef().GetData() )
	{
		//	update
		case TRef_Static(U,p,d,a,t)://TLCore::UpdateRef:
			Update();
			break;

		//	new peer connected
		case TRef_Static(P,R,C,o,n):
		{
			TRef PeerRef;
			if ( Message.ImportData( TRef_Static4(P,e,e,r), PeerRef ) )
			{
				OnPeerConnected( PeerRef );
			}
		}
		break;

		//	peer disconnected 
		case TRef_Static(P,R,D,i,s):
		{
			TRef PeerRef;
			if ( Message.ImportData( TRef_Static4(P,e,e,r), PeerRef ) )
			{
				OnPeerDisconnected( PeerRef );
			}
		}
		break;

		//	incoming data
		case TRef_Static(O,n,P,c,k):
		{
			//	get who the data has come from
			TRef PeerRef;
			TFtpClient* pClient = NULL;
			if ( Message.ImportData( TRef_Static4(P,e,e,r), PeerRef ) )
				pClient = m_FtpClients.FindPtr( PeerRef );

			//	get the data
			TBinaryTree* pPacketData = Message.GetChild( TRef_Static4( D,a,t,a ) );
			if ( pClient && pPacketData )
			{
				//	handle ftp packet
				OnPacket( pPacketData->GetData(), *pClient );
			}
		}
		break;
	};
}

//-------------------------------------------------------
//	new peer connected - setup ftp client
//-------------------------------------------------------
void TLReflection::TFileServer::OnPeerConnected(TRefRef PeerRef)
{
	//	check if we already have an entry
	TPtr<TFtpClient> pClient = m_FtpClients.FindPtr( PeerRef );
	if ( pClient )
	{
		TLDebug_Break("peer connected... but client already exists...");
	}

	//	create new client
	pClient = new TFtpClient( PeerRef );

	//	add entry
	m_FtpClients.Add( PeerRef, pClient );

	//	send welcome message
	TTempString WelcomeString("Hello");
	pClient->SendPacket( 220, WelcomeString, *m_pServer );
}

//-------------------------------------------------------
//	peer disconnected - remove client
//-------------------------------------------------------
void TLReflection::TFileServer::OnPeerDisconnected(TRefRef PeerRef)
{
	//	remove ftp client (this should clean up everything for us)
	m_FtpClients.Remove( PeerRef );
}

//-------------------------------------------------------
//	this client sent a new ftp packet
//-------------------------------------------------------
void TLReflection::TFileServer::OnPacket(TBinary& Packet,TFtpClient& Client)
{
	//	turn the entire contents into a string
	Packet.ResetReadPos();
	char* pPacketAnsi = Packet.ReadNoCopy<char>();
	TString PacketString;
	PacketString.Append( pPacketAnsi, Packet.GetSize() );

	TDebugString Debug_String;
	Debug_String << "Ftp incoming: \"" << PacketString << "\"";
	TLDebug_Print( Debug_String );

	//	split into multiple commands
	//	gr: note, we're supposed to split at explicit \r\n, not \r OR \n... this may be an issue...
	TFixedArray<TChar,2> SplitChars;
	SplitChars << '\r' << '\n';
	TArray<TString> CommandStrings;
	PacketString.Split( SplitChars, CommandStrings );
	
	//	put the commands into the client
	for ( u32 c=0;	c<CommandStrings.GetSize();	c++ )
	{
		TString& CommandString = CommandStrings[c];
		
		//	split command from argument
		TFixedArray<TString,2> CommandParts;
		CommandString.Split(' ', CommandParts);

		//	gr: the commands are always 4 or less characters, so turn it into a ref for easier coding
		//	convert args based on split results to handle empty strings (will give an error)
		//	or commands with no args
		TRef CommandRef = (CommandParts.GetSize()>=1) ? TRef( CommandParts[0] ) : TRef( CommandString );

		TTempString DummyString;
		TString& CommandArgs = (CommandParts.GetSize()>=2) ? CommandParts[1] : DummyString;

		//	add command to client to process
		Client.AddPendingCommand( CommandRef, CommandArgs );
	}

	//	execute any/all the commands we've put onto the client
	Client.ExecuteCommands( *m_pServer );
}



TLReflection::TFtpClient::TFtpClient(TRefRef PeerRef) :
	m_PeerRef		( PeerRef ),
	m_PassiveMode	( false )
{
}


//----------------------------------------------------------------------------//
//	update routine, required to catch the finishing of a data transfer
//----------------------------------------------------------------------------//
void TLReflection::TFtpClient::Update(TLNetwork::TConnectionServerSocket& Server)
{
	if ( m_pDataTask )
	{
		SyncBool TaskState = m_pDataTask->GetState();
		if ( TaskState == SyncTrue )
		{
			OnDataTaskFinished( true, Server );
		}
		else if ( TaskState == SyncFalse )
		{
			OnDataTaskFinished( false, Server );
		}
	}
}


//----------------------------------------------------------------------------//
//	send ftp packet to server
//----------------------------------------------------------------------------//
bool TLReflection::TFtpClient::SendPacket(u32 FtpCommand,const TString& String,TLNetwork::TConnectionServerSocket& Server)
{
	//	put the command, the string and a line terminator into a packet
	TString FinalString;
	FinalString << FtpCommand << ' ' << String << "\r\n";

	//	convert to ansi for sending
	TArray<char> AnsiFinalString;

	//	note: no terminator! very important! the last characters HAVE to be <crlf> other wise clients won't process the command
	FinalString.GetAnsi( AnsiFinalString, false );

	TBinary PacketData;
	PacketData.WriteData( AnsiFinalString );

	return Server.SendToPeer( PacketData, GetPeerRef() );
}


//----------------------------------------------------------------------------//
//	execute any/all the commands we've put onto the client. return false if we want to disconnect the client
//----------------------------------------------------------------------------//
bool TLReflection::TFtpClient::ExecuteCommands(TLNetwork::TConnectionServerSocket& Server)
{
	while ( m_CommandQueue.GetSize() > 0 )
	{
		TFtpCommand& Command = *m_CommandQueue[0];
		
		//	if false, then error or quit
		if ( !ExecuteCommand( Command, Server ) )
			return false;

		//	remove this processed command
		m_CommandQueue.RemoveAt( 0 );
	}

	//	keep connection alive
	return true;
}

//----------------------------------------------------------------------------//
//	
//----------------------------------------------------------------------------//
bool TLReflection::TFtpClient::SendPacketCurrentDir(u32 FtpCommand,TLNetwork::TConnectionServerSocket& Server)
{
	TTempString CurrentDirString;

	if ( m_FileSystem.IsValid() )
		CurrentDirString << '"' << m_FileSystem << '"' << " is current dir/file system";
	else
		CurrentDirString << '"' << '/' << '"' << " (Currently in root)";

	return SendPacket( FtpCommand, CurrentDirString, Server );
}


//----------------------------------------------------------
//	get 'ls' output for a file/dir
//----------------------------------------------------------
void ls(TString& Output,const TString& FileName,bool IsDir,bool IsWritable,u32 Size,const TLTime::TTimestamp& Timestamp)
{
	//	drwx------   3 slacker    users         104 Jul 27 01:45 public_html

	//	append permissions
	Output << (IsDir ? 'd':'-' ) 
			<< 'r' 
			<< (IsWritable ? 'w':'-' )
			<< "x------";

	// groups
	Output << " 1 TootleUser TootleGroup";

	//	file size
	Output << ' ' << Size;

	//	timestamp
	//	todo;
	Output << " Jan 1 02:03";

	//	file/dir name
	Output << ' ' << FileName;
}

//----------------------------------------------------------------------------//
//	get a directory listing string
//----------------------------------------------------------------------------//
bool TLReflection::TFtpClient::GetDirectoryList(const TString& Arguments,TString& DirListing,bool JustNames) const
{
	//	resolve directory
	TRef FileSystem = m_FileSystem;

	TPointerArray<TLFileSys::TFileSys> ListFileSystems;
	TPointerArray<TLFileSys::TFile> ListFiles;

	if ( !Arguments.IsEmpty() )
	{
		//	todo: extract path and/or file from args
	}


	//	list root (list of file systems)
	if ( !FileSystem.IsValid() )
	{
		TPtrArray<TLFileSys::TFileSys>& FileSysList = TLFileSys::GetFileSystems();
		for ( u32 fs=0;	fs<FileSysList.GetSize();	fs++ )
		{
			ListFileSystems.Add( FileSysList[fs] );
		}
	}
	else
	{
		//	list files inside file system
		const TLFileSys::TFileSys* pFileSystem = TLFileSys::GetFileSys( m_FileSystem );
		if ( !pFileSystem )
			return false;

		//	get files
		//	gr; cast to use const version
		const TPtrArray<TLFileSys::TFile>& FileList = pFileSystem->GetFileList();
		for ( u32 f=0;	f<FileList.GetSize();	f++ )
		{
			ListFiles.Add( FileList[f] );
		}
	}

	//	gr; may need to vary theis between LF and CRLF as the binary/ascii mode dictates...
	const char* LineFeed = "\r\n";

	//	out put our lists into a string
	u32 i;
	for ( i=0;	i<ListFileSystems.GetSize();	i++ )
	{
		TLFileSys::TFileSys& FileSys = *ListFileSystems[i];

		//	just get name of dir
		if ( JustNames )
		{
			DirListing << FileSys.GetFileSysRef();
		}
		else
		{
			//	get ls-style output
			TTempString DirName;
			DirName << FileSys.GetFileSysRef();
			ls( DirListing, DirName, true, FileSys.IsWritable(), 0, FileSys.GetTimestamp() );
		}

		DirListing << LineFeed;
	} 

	for ( i=0;	i<ListFiles.GetSize();	i++ )
	{
		TLFileSys::TFile& File = *ListFiles[i];

		//	just get name of dir
		if ( JustNames )
		{
			DirListing << File.GetFilename();
		}
		else
		{
			//	get ls-style output
			ls( DirListing, File.GetFilename(), false, File.IsWritable(), File.GetSize(), File.GetTimestamp() );
		}

		DirListing << LineFeed;
	}

	return true;
}

//----------------------------------------------------------------------------//
//	execute a command. return false if we want to disconnect the client
//----------------------------------------------------------------------------//
bool TLReflection::TFtpClient::ExecuteCommand(const TFtpCommand& Command,TLNetwork::TConnectionServerSocket& Server)
{
	/*
	case TRef_Static4( 
		if ((m_nStatus == STATUS_LOGIN) && (strCommand != "USER" && strCommand != "PASS"))
	{
		SendResponse("530 Please login with USER and PASS.");
		return;
	}
	*/

	switch ( Command.m_Ref.GetData() )
	{
		// username entered
		case TRef_Static4(U,S,E,R):
		{
			/*
			m_nStatus = STATUS_LOGIN;
			m_strUserName = strArguments;

			TString strPeerAddress;
			UINT nPeerPort;
			GetPeerName(strPeerAddress, nPeerPort);

			// tell FTP server a new user has connected
			CConnectThread *pThread = (CConnectThread *)m_pThread;
			((CFTPServer *)pThread->m_pWndServer)->m_pEventSink->OnFTPUserConnected(m_pThread->m_nThreadID, m_strUserName, strPeerAddress);

			SendResponse("331 Password required for %s", m_strUserName);
			*/
			//	copy username
			m_Username = Command.m_Arguments;

			TTempString Reply;
			Reply << "Don't really required a pass thanks " << m_Username;
			return SendPacket(331, Reply, Server );
		}
		break;

		// password entered
		case TRef_Static4(P,A,S,S):
		{
			//	ask for login
			if ( m_Username.IsEmpty() )
			{
				SendPacket( 503, "Login with USER first.", Server );
				return true;
			}

			// now we have user name and password, attempt to login the client
			//	if ( invalid pass )
			if ( false )
			{
				SendPacket( 530, "Not logged in, user/password invalid", Server );
				return true;
			}

			//	set home dir...
			m_FileSystem = TRef();
			return SendPacket( 230, "User logged in.", Server );
		}
		break;

		// close connection
		case TRef_Static4(Q,U,I,T):
		case TRef_Static3(B,Y,E):
		{
			// send goodbye message to client
			SendPacket( 220, "good bye!", Server );
			return false;
		}
		break;

		// change transfer type
		case TRef_Static4(T,Y,P,E):
		{
			// let's pretend we did something...
			return SendPacket( 200, "Pretended to change type ok", Server );
		}
		break;

		// print current directory
		case TRef_Static3(P,W,D):
		{
			return SendPacketCurrentDir( 250, Server );
		}
		break;

		// change to parent directory
		case TRef_Static4(C,D,U,P):
		{
			//	already at root
			if ( !m_FileSystem.IsValid() )
				return SendPacket( 550, "Already at root", Server );

			//	move up
			m_FileSystem.SetInvalid();
			return SendPacketCurrentDir( 250, Server );
		}
		break;
	
		// change working directory
		case TRef_Static3(C,W,D):
		{
			TRef NewDir;

			//	change-to-root commands
			if ( Command.m_Arguments == ".." )
			{
				NewDir = TRef();
			}
			else
			{
				//	a path will typically come in as /path/nextpath/ (or just / for root) so split it up into individual directories...
				//	as we only ever have 1 path, we will split into just 2 strings and auto-fail if an extra directory has been provided
				TFixedArray<TTempString,2> PathDirs;
				Command.m_Arguments.Split('/', PathDirs );

				//	when splitting a string, if the splitter is the first char, we get an empty string (see TString::Split) so remove it
				//	if we have one to save confusion
				if ( PathDirs.GetSize() > 0 )
				{
					if ( PathDirs[0].IsEmpty() )
						PathDirs.RemoveAt( 0 );
				}

				//	find file sys
				TLFileSys::TFileSys* pFileSys = NULL;

				//	no dirs? presume it was just / as the path then, so we want to change dir to the root
				if ( PathDirs.GetSize() == 0 )
				{
					NewDir = TRef();
				}
				else if ( PathDirs.GetSize() == 1 )
				{
					//	one dir specified (a file sys) so get the file sys based on the dir
					NewDir = PathDirs[0];
					
					//	fetch file sys
					pFileSys = TLFileSys::GetFileSys( NewDir );
				}
				else
				{
					//	multiple paths specified, we don't support this... let it fail
					//	use 2Many as the path for debugging, also so we give out the "doesn't exist" error back to the client
					NewDir = "2Many";	//	"2ManyArgs"
				}

				//	we were given a dir, but it doesn't exist
				if ( !pFileSys && NewDir.IsValid() )
				{
					TTempString ReplyString;
					ReplyString << "File system \"" << Command.m_Arguments << "\" (" << NewDir << ") doesn't exist";
					return SendPacket( 550, ReplyString, Server );
				}
				else
				{
					//	file sys exists, or we're using root... continue to change dir below...
				}
			}

			//	change dir
			m_FileSystem = NewDir;
			return SendPacketCurrentDir( 250, Server );
		}
		break;
	
		/* gr: only supporting passive mode atm
		// specify IP and port (PORT a1,a2,a3,a4,p1,p2) -> IP address a1.a2.a3.a4, port p1*256+p2. 
		case TRef_Static4(P,O,R,T):
		{
			TString strSub;
			int nCount=0;

			while (AfxExtractSubString(strSub, strArguments, nCount++, ','))
			{
				switch(nCount)
				{
					case 1:	// a1
						m_strRemoteHost = strSub;
						m_strRemoteHost += ".";
						break;
					case 2:	// a2
						m_strRemoteHost += strSub;
						m_strRemoteHost += ".";
						break;
					case 3:	// a3
						m_strRemoteHost += strSub;
						m_strRemoteHost += ".";
						break;
					case 4:	// a4
						m_strRemoteHost += strSub;
						break;
					case 5:	// p1
						m_nRemotePort = 256*atoi(strSub);
						break;
					case 6:	// p2
						m_nRemotePort += atoi(strSub);
						break;
				}
			}
			SendResponse("200 Port command successful.");
		}
		break;
		*/

		// switch to passive mode
		case TRef_Static4(P,A,S,V):
		{
			//	create data server
			if ( !CreateDataServer() )
			{
				//SendPacket( 425, "425 Can't open data connection.", Server );
				return SendPacket( 421, "Failed to create data socket", Server );
			}

			//	set passive mode
			m_PassiveMode = true;

			//	get data address & port
			TLNetwork::TAddress DataAddress = m_pDataServer->GetAddress();
			if ( !DataAddress.IsValid() )
				return false;

			//	format of PORT is ip0,ip1,ip2,ip3,portblock,portremainder
			//	the port is divided into blocks of 256, so port 1,2 is (1*256)+2

			//	tell the client which address/port to connect to
			TTempString Reply;
			const Type4<u8>& IP = DataAddress.GetIP();
			Reply << "Entering passive mode (" << IP.x <<','<< IP.y <<','<< IP.z <<','<< IP.w <<','<< DataAddress.GetPort()/256 <<','<< DataAddress.GetPort()%256 << ").";
			return SendPacket( 227, Reply, Server );
		} 
		break;

		// list current directory (or a specified file/directory)
		case TRef_Static4(N,L,S,T):
		case TRef_Static4(L,I,S,T):
		{
			//	only supported via passive mode. so if the client (eg. windows ftp cli!) hasn't
			//	setup PORT or PASV mode, we fail
			if ( !this->m_PassiveMode )
			{
				return SendPacket( 425, "Client hasn't specified port or pasv mode...", Server );
			}

			TString DirListing;
			bool JustNames = (Command.m_Ref == TRef_Static4(N,L,S,T) );
			if ( !GetDirectoryList( Command.m_Arguments, DirListing, JustNames ) )
			{
				// something went wrong
				return false;
			}

			//	start dir-listing data transfer
			if ( !SendDataDirListing( DirListing, Server ) )
				return SendPacket( 425, "Cannot open data connection.", Server );

			return SendPacket( 150, "Opening ASCII mode data connection for directory list.", Server ); 
		} 
		break;


		// retrieve file
		case TRef_Static4(R,E,T,R):
		{
			//	get file
			TLFileSys::TFileSys* pFileSys = TLFileSys::GetFileSys( m_FileSystem );
			TPtr<TLFileSys::TFile>& pFile = pFileSys ? pFileSys->GetFile( Command.m_Arguments ) : TLPtr::GetNullPtr<TLFileSys::TFile>();
			if ( !pFile )
			{
				TTempString ReplyString;
				ReplyString << "File not found: \"" << Command.m_Arguments << "\"";
				return SendPacket( 550, ReplyString, Server );
			}

			//	start sending file
			if ( !SendDataSendFile( pFile, Server ) )
				return false;

			//	send response
			return SendPacket( 150, "Opening BINARY mode data connection for file transfer.", Server );
		}
		break;

		// client wants to upload file
		case TRef_Static4(S,T,O,R):
		{
			const TString& Filename = Command.m_Arguments;
			TLFileSys::TFileSys* pFileSys = TLFileSys::GetFileSys( m_FileSystem );
			TPtr<TLFileSys::TFile> pNewFile = pFileSys ? pFileSys->CreateNewFile( Filename ) : NULL;

			//	failed to create file...
			if ( !pNewFile )
				return SendPacket( 550, "Failed to create new file", Server );

			//	start data connection to retrieve new file
			if ( !SendDataRecieveFile( pNewFile, Server ) )
				return false;

			//	send confirmation
			return SendPacket( 150, "Opening BINARY mode data connection for file transfer.", Server );
		}
		break;

		// get file size
		case TRef_Static4(S,I,Z,E):
		{
			//	get file from current file system
			const TString& Filename = Command.m_Arguments;
			TLFileSys::TFileSys* pFileSys = TLFileSys::GetFileSys( m_FileSystem );
			TLFileSys::TFile* pFile = pFileSys ? pFileSys->GetFile( Filename ) : NULL;

			//	no such file/sys
			if ( !pFile )
				return SendPacket( 550, "File not found", Server );

			//	return file size
			TTempString ReplyString;
			ReplyString << pFile->GetFileSize();
			return SendPacket( 213, ReplyString, Server );
		}
		break;
		
		// delete file
		case TRef_Static4(D,E,L,E):
		{
			//	get file from current file system
			const TString& Filename = Command.m_Arguments;
			TLFileSys::TFileSys* pFileSys = TLFileSys::GetFileSys( m_FileSystem );

			//	if file sys is read only, send permission denied
			if ( pFileSys && !pFileSys->IsWritable() )
				return SendPacket( 550, "Permission denied", Server );

			TPtr<TLFileSys::TFile> pFile = pFileSys ? pFileSys->GetFile( Filename ) : TLPtr::GetNullPtr<TLFileSys::TFile>();
			
			//	no such file/sys
			if ( !pFile )
				return SendPacket( 550, "File not found", Server );

			//	gr: hack until i sort out includes...
#ifdef DeleteFile
#undef DeleteFile
#endif

			//	delete file
			if ( !pFileSys->DeleteFile( pFile ) )
				return SendPacket( 450, "Internal error deleting file", Server );

			//	success
			return SendPacket( 250, "file deleted", Server );
		}
		break;

		// remove directory
		case TRef_Static3(R,M,D):
			//	cannot delete file systems
			return SendPacket( 550, "Cannot delete file systems", Server );

		// create directory
		case TRef_Static3(M,K,D):
			//	cannot create file systems
			return SendPacket( 550, "Cannot create file systems", Server );

		// abort transfer
		case TRef_Static4(A,B,O,R):
		{
			//	destroy data server
			if ( m_pDataServer )
			{
				//	destroy current task
				if ( m_pDataTask )
				{
					m_pDataTask = NULL;
					SendPacket( 426, "Data connection closed.", Server );
				}

				//	destroy connection
				DestroyDataServer();
			}
			return SendPacket( 226, "ABOR command successful.", Server );
		} 
		break;

		// get system info
		case TRef_Static4(S,Y,S,T):
			return SendPacket( 215, "UNIX emulated - Tootle Game runtime file system access", Server );

		//	dummy instruction (keep alive)
		case TRef_Static4(N,O,O,P):
			return SendPacket( 200, "OK", Server );

		default:
		{
			TTempString Reply;
			Reply << "Command \"" << Command.m_Ref << "\" not implemented.";
			return SendPacket( 502, Reply, Server );
		}
	}
}


//----------------------------------------------------------
//	create data server (this gets pre-created for PASV mode)
//----------------------------------------------------------
bool TLReflection::TFtpClient::CreateDataServer()
{
	//	already have a data socket
	if ( m_pDataServer )
		return true;

	//	create new socket
	TBinaryTree ServerInfo( TRef("Data") );
	ServerInfo.ExportData("Protocol", TRef("TCP") );
	ServerInfo.ExportData("Port", (u16)g_Port-1 );		//	gr; todo: make this automatic!
	m_pDataServer = new TLNetwork::TConnectionServerSocket( ServerInfo );

	//	failed alloc?
	if ( !m_pDataServer )
		return false;

	//	initialise
	TRef Error;
	SyncBool Result = SyncWait;

	//	gr: need a better async system!
	while ( Result == SyncWait )
	{
		Result = m_pDataServer->Initialise(Error);
	}

	//	fail
	if ( Result == SyncFalse )
	{
		DestroyDataServer();
		return false;
	}

	//	subscribe to server to get connection/data messages
	//	gr: note, this MIGHT be a little too late depending on the sockets implementation!
	this->SubscribeTo( m_pDataServer );

	//	success
	return true;
}

//----------------------------------------------------------
//	destroy, close and clean up the data server
//----------------------------------------------------------
void TLReflection::TFtpClient::DestroyDataServer()
{
	DestroyDataTask();
	m_pDataServer = NULL;
}

//----------------------------------------------------------
//	destroy and clean up the data task
//----------------------------------------------------------
void TLReflection::TFtpClient::DestroyDataTask()
{
	//	peer relates to task
	m_pDataTask = NULL;
	m_DataPeer.SetInvalid();
}


//----------------------------------------------------------
//	create a data server to transfer something
//----------------------------------------------------------
bool TLReflection::TFtpClient::CreateDataTask(TPtr<TFtpDataTask>& pNewTask,TLNetwork::TConnectionServerSocket& CommandServer)
{
	if ( !pNewTask )
	{
		TLDebug_Break("Task expected");
		return false;
	}

	//	delete existing task...
	m_pDataTask = NULL;

	//	expect a data server to have already been created
	if ( !m_pDataServer )
	{
		TLDebug_Break("Expected existing data server");
		return false;
	}

	//	set new task
	m_pDataTask = pNewTask;

	//	peer is already connected to data port, so set that up
	if ( m_DataPeer.IsValid() )
	{
		m_pDataTask->OnPeerConnected( m_pDataServer, m_DataPeer );
	}

	//	get initial state of the task
	SyncBool State = m_pDataTask->GetState();

	//	if initial state is waiting, then it was a success initialising
	if ( State == SyncWait )
		return true;

	//	if it finished immediately, then destroy the task
	bool Success = (State == SyncTrue);

	//	notify result
	OnDataTaskFinished( Success, CommandServer );

	//	destroy task
	m_pDataTask = NULL;

	return Success;
}


//----------------------------------------------------------
//	send dir listing via data connection
//----------------------------------------------------------
bool TLReflection::TFtpClient::SendDataDirListing(const TString& DirListing,TLNetwork::TConnectionServerSocket& CommandServer)
{
	//	create data socket
	if ( !CreateDataServer() )
		return false;

	TPtr<TFtpDataTask> pNewTask = new TFtpDataTaskDirListing( DirListing );
	return CreateDataTask( pNewTask, CommandServer );
}


//----------------------------------------------------------
//	send file via data connection
//----------------------------------------------------------
bool TLReflection::TFtpClient::SendDataSendFile(TPtr<TLFileSys::TFile>& pFile,TLNetwork::TConnectionServerSocket& CommandServer)
{
	//	create data socket
	if ( !CreateDataServer() )
		return false;

	TPtr<TFtpDataTask> pNewTask = new TFtpDataTaskSendFile( pFile );
	return CreateDataTask( pNewTask, CommandServer );
}


//----------------------------------------------------------
//	recieve into this file via data connection
//----------------------------------------------------------
bool TLReflection::TFtpClient::SendDataRecieveFile(TPtr<TLFileSys::TFile>& pNewFile,TLNetwork::TConnectionServerSocket& CommandServer)
{
	//	create data socket
	if ( !CreateDataServer() )
		return false;

	TPtr<TFtpDataTask> pNewTask = new TFtpDataTaskRecieveFile( pNewFile );
	return CreateDataTask( pNewTask, CommandServer );
}


//----------------------------------------------------------
//	current data task has finished
//----------------------------------------------------------
void TLReflection::TFtpClient::OnDataTaskFinished(bool Success,TLNetwork::TConnectionServerSocket& CommandServer)
{
	//	clean up task
	DestroyDataServer();

	//	send confirmation
	if ( Success )
	{
		SendPacket( 226, "Transfer complete.", CommandServer );
	}
	else
	{
		SendPacket( 426, "Connection closed; transfer aborted.", CommandServer );
	}
}


//----------------------------------------------------------------------------
//	
//----------------------------------------------------------------------------
void TLReflection::TFtpClient::ProcessMessage(TLMessaging::TMessage& Message)
{
	//	handle message from socket
	switch ( Message.GetMessageRef().GetData() )
	{
		//	new peer connected
		case TRef_Static(P,R,C,o,n):
		{
			TRef PeerRef;
			if ( Message.ImportData( TRef_Static4(P,e,e,r), PeerRef ) )
			{
				//	already have a peer?
				if ( m_DataPeer.IsValid() )
				{
					TDebugString Debug_String;
					Debug_String << "Peer (" << PeerRef << ") connected, already have a peer (" << m_DataPeer << ")... what to do here? kill task of old peer?";
					TLDebug_Print( Debug_String );
					m_pDataTask = NULL;
				}

				//	store peer connected to data socket
				m_DataPeer = PeerRef;

				//	notify task 
				//	note; this may not exist yet! client may be connecting to data port in anticipation of transfer
				if ( m_pDataTask )
				{
					m_pDataTask->OnPeerConnected( m_pDataServer, m_DataPeer );
				}
			}
		}
		break;

		//	peer disconnected
		case TRef_Static(P,R,D,i,s):
		{
			bool GracefullDisconnect = false;
			Message.ImportData( TRef_Static(G,r,a,c,e), GracefullDisconnect );

			TRef PeerRef;
			if ( Message.ImportData( TRef_Static4(P,e,e,r), PeerRef ) )
			{
				//	if this is our data peer, then close the data server
				if ( PeerRef == m_DataPeer )
				{
					//	notify task first 
					if ( m_pDataTask )
					{
						m_pDataTask->OnPeerDisconnected( GracefullDisconnect );
					}

					//	gr: don't destroy the data server here, this message will have come via the data server
					//		so if we delete it we'll break the callstack (well, we'll break when we get out of this func)
					//		instead the data server is just going to sit there doing nothing.
					//DestroyDataServer();
					//	gr: don't destroy the task either. we need to wait for the update loop from the ftpserver (parent of this client)
					//		to confirm back to the client via the control that the task has succeeded or failed, and some times we wait 
					//		for the disconnection before we have an outcome...
					if ( m_pDataTask )
					{
						SyncBool TaskState = m_pDataTask->GetState();
						if ( TaskState == SyncWait )
						{
							TLDebug_Break("Upon disconnection, the task state MUST know whether it has succeeded or failed. Forcing to fail");
							m_pDataTask->OnError();
						}
					}
					//	gr: invalidate the data peer anyway so we know they're not connected. Will this cause a problem?
					m_DataPeer.SetInvalid();
				}
				else
				{
					TDebugString Debug_String;
					const char* pGracefullyDisconnectedString = GracefullDisconnect ? "Gracefully" : "badly";
					Debug_String << "Peer (" << PeerRef << ") disconnected (" << pGracefullyDisconnectedString << ") but is not our data peer (" << m_DataPeer << ").";
					TLDebug_Print( Debug_String );
				}
			}
		}
		break;

		//	incoming data
		case TRef_Static(O,n,P,c,k):
		{
			//	get the data
			TBinaryTree* pPacketData = Message.GetChild( TRef_Static4( D,a,t,a ) );
			if ( pPacketData )
			{
				//	error if we get this too early!
				if ( !m_pDataTask )
				{
					//	todo: send error back to client?
					TLDebug_Break("Packet too early on data socket");
				}
				else
				{
					//	handle incoming packet for task
					m_pDataTask->OnPacketRecieved( pPacketData->GetData() );
				}
			}
		}
		break;

		//	packet sent
		case TRef_Static(O,n,S,e,n):
		{
			TRef PacketRef;
			if ( Message.Read( PacketRef ) )
			{
				//	notify task that packet was sent
				if ( m_pDataTask )
					m_pDataTask->OnPacketSent( PacketRef );
			}
		}
		break;

	};
}


//----------------------------------------------------------------------------
//	
//----------------------------------------------------------------------------
TLReflection::TFtpDataTask::TFtpDataTask() :
	m_PacketSent	( SyncWait )
{
}

//----------------------------------------------------------------------------
//	get state of the task - SyncTrue finished(or cancelled), SyncWait.. still going, SyncFalse... failed
//----------------------------------------------------------------------------
SyncBool TLReflection::TFtpDataTask::GetState()
{
	//	if no socket yet, then still waiting for peer to connect
	if ( !m_pSocket )
		return SyncWait;

	//	if the queued packet has been sent or failed then return
	if ( m_PacketSent != SyncWait )
		return m_PacketSent;

	//	base state is okay to continue...
	return SyncTrue;
}



//----------------------------------------------------------------------------
//	our client has connected, send out the queued packet (if there is one)
//----------------------------------------------------------------------------
void TLReflection::TFtpDataTask::OnPeerConnected(TPtr<TLNetwork::TConnectionServerSocket>& pSocket,TRefRef Peer)
{
	//	store connection info
	m_pSocket = pSocket;
	m_PeerRef = Peer;

	//	send out the queued packet
	if ( m_pSendPacketData )
	{
		//	fail if packet couldn't be sent
		if ( !m_pSocket->SendToPeer( *m_pSendPacketData, m_PeerRef, TRef_Static4(D,a,t,a) ) )
			m_PacketSent = SyncFalse;
	}
}

//----------------------------------------------------------------------------
//	queue up a packet to be sent when the peer connects
//----------------------------------------------------------------------------
void TLReflection::TFtpDataTask::SendPacket(TBinary& PacketData)
{
	//	already have a packet
	if ( m_pSendPacketData )
	{
		TLDebug_Break("Packet already queued to be sent out... replacing");
	}

	//	queue up packet
	m_pSendPacketData = new TBinary( PacketData );

	//	resest queue state, though I don't thiunk should have changed from syncwait
	m_PacketSent = SyncWait;

	//	already have a peer, so send out immediately, or it won't get sent
	if ( m_PeerRef.IsValid() && m_pSocket )
		OnPeerConnected( m_pSocket, m_PeerRef );
}



//----------------------------------------------------------------------------
//	
//----------------------------------------------------------------------------
TLReflection::TFtpDataTaskDirListing::TFtpDataTaskDirListing(const TString& DirListing)
{
	//	convert to ansi string
	TArray<char> AnsiListing;
	DirListing.GetAnsi( AnsiListing );

	//	send data
	TBinary PacketData;
	PacketData.WriteData( AnsiListing );
	SendPacket( PacketData );
}



//----------------------------------------------------------------------------
//	
//----------------------------------------------------------------------------
TLReflection::TFtpDataTaskSendFile::TFtpDataTaskSendFile(TPtr<TLFileSys::TFile>& pFile)
{
	if ( !pFile )
	{
		TLDebug_Break("File expected");
		OnError();
		return;
	}

	//	gr: todo: make this more asynchronous for when async file loading is implemented
	SyncBool IsFileLoaded = pFile->IsLoaded();
	if ( IsFileLoaded != SyncTrue )
	{
		//	load the file from it's file sys
		TLFileSys::TFileSys* pFileSys = pFile->GetFileSys();
		if ( !pFileSys )
		{
			TLDebug_Break("File system of file expected. Cannot load, in order to send.");
			OnError();
			return;
		}

		//	load file from file sys
		IsFileLoaded = pFileSys->LoadFile( pFile );

		//	failed to load
		if ( IsFileLoaded != SyncTrue )
		{
			if ( IsFileLoaded == SyncWait )
			{
				TLDebug_Break("Cannot handle sync loading files atm");
			}

			OnError();
			return;
		}
	}

	//	send data
	TBinary PacketData;
	PacketData.WriteData( pFile->GetData() );
	SendPacket( PacketData );
}


//----------------------------------------------------------------------------
//	task to recieve file from peer
//----------------------------------------------------------------------------
TLReflection::TFtpDataTaskRecieveFile::TFtpDataTaskRecieveFile(TPtr<TLFileSys::TFile>& pNewFile) :
	m_pRecievingFile	( pNewFile )
{
	if ( !m_pRecievingFile )
	{
		TLDebug_Break("File expected");
		return;
	}

	//	empty the new file and mark as loading
	m_pRecievingFile->SetIsLoaded( SyncWait );
	m_pRecievingFile->Empty();
}


//----------------------------------------------------------------------------
//	get state of the task - SyncTrue finished(or cancelled), SyncWait.. still going, SyncFalse... failed
//----------------------------------------------------------------------------
SyncBool TLReflection::TFtpDataTaskRecieveFile::GetState()
{
	SyncBool BaseResult = TFtpDataTask::GetState();
	if ( BaseResult != SyncTrue )
		return BaseResult;

	//	invalid setup
	if ( !m_pRecievingFile )
		return SyncFalse;

	//	wait until we have a finished state for the file
	SyncBool FileState = m_pRecievingFile->IsLoaded();

	return FileState;
}



//----------------------------------------------------------------------------
//	recieved packet from data server
//----------------------------------------------------------------------------
void TLReflection::TFtpDataTaskRecieveFile::OnPacketRecieved(TBinary& Data)
{
	if ( !m_pRecievingFile )
	{
		TLDebug_Break("Recieving file expected");
		return;
	}

	//	write data into file
	m_pRecievingFile->GetData().WriteData( Data );

	//	gr: end of file is dictated by closing the connection or with an EOF marker. 
	//		something like that. need to test more (make sure testing with file >2k! thats the buffer limit! a packet will only ever be 2k!)
}



//----------------------------------------------------------------------------
//	recieved packet from data server
//----------------------------------------------------------------------------
void TLReflection::TFtpDataTaskRecieveFile::OnPeerDisconnected(bool Gracefully)
{
	if ( !m_pRecievingFile )
	{
		TLDebug_Break("Recieving file expected");
		return;
	}

	//	if the peer disconnected badly, then change the file state to false to indicate the current
	//	file state is invalid
	//	gr: close the file here for future fs streaming changes? and to lose the bad data?
	if ( !Gracefully )
	{
		m_pRecievingFile->SetIsLoaded( SyncFalse );
		return;
	}

	//	file has been recieved, finalise the state of the file
	m_pRecievingFile->OnFileLoaded();

	//	write file back to the file system to commit it back onto the real file system (eg. HDD)
	SyncBool WriteResult = SyncFalse;
	TLFileSys::TFileSys* pFileSys = m_pRecievingFile->GetFileSys();
	if ( !pFileSys )
	{
		TLDebug_Break("File system of file expected. Cannot write back to hard disk after reciveing file.");
		WriteResult = SyncFalse;
	}
	else
	{
		//	write file back to file sys
		WriteResult = pFileSys->WriteFile( m_pRecievingFile );
	}

	//	manually set the file as "changed" to be caught by the asset-reloading system
	m_pRecievingFile->SetOutOfDate( true );

	//	handle post-writing of file
	if ( WriteResult == SyncWait )
	{
		TLDebug_Print("todo: handle non-instant re-write back to file system");
		return;
	}
}


