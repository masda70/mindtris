
#include "mindtriscore/includes.h"
#include "mindtriscore/util.h"
#include "mindtriscore/socket.h"
#include "mindtriscore/serverprotocol.h"
#include "mindtriscore/p2pprotocol.h"
#include "peer.h"
#include "mindtris.h"

#include <locale>

#ifdef WIN32
 #include "curses.h"
#else
 #include <curses.h>
#endif


bool gCurses = false;
vector<string> gMainBuffer;
string gInputBuffer;
string gLobbyName;
vector<string> gLobbyPeerNames;
WINDOW *gMainWindow = NULL;
WINDOW *gBottomBorder = NULL;
WINDOW *gRightBorder = NULL;
WINDOW *gInputWindow = NULL;
WINDOW *gLobbyWindow = NULL;
bool gMainWindowChanged = false;
bool gInputWindowChanged = false;
bool gLobbyWindowChanged = false;


void CONSOLE_ChangeLobby( string lobby )
{
	gLobbyName = lobby;
	gLobbyWindowChanged = true;
}

void CONSOLE_AddLobbyPeer( string name )
{
	gLobbyPeerNames.push_back( name );
	gLobbyWindowChanged = true;
}

void CONSOLE_RemoveLobbyPeer( string name )
{
	for( vector<string> :: iterator i = gLobbyPeerNames.begin( ); i != gLobbyPeerNames.end( ); )
	{
		if( *i == name )
			i = gLobbyPeerNames.erase( i );
		else
			i++;
	}

	gLobbyWindowChanged = true;
}


void CONSOLE_Draw( )
{
	if( !gCurses )
		return;

	// draw main window

	if( gMainWindowChanged )
	{
		wclear( gMainWindow );
		wmove( gMainWindow, 0, 0 );

		for( vector<string> :: iterator i = gMainBuffer.begin( ); i != gMainBuffer.end( ); i++ )
		{
			for( string :: iterator j = (*i).begin( ); j != (*i).end( ); j++ )
				waddch( gMainWindow, *j );

			if( i != gMainBuffer.end( ) - 1 )
				waddch( gMainWindow, '\n' );
		}

		wrefresh( gMainWindow );
		gMainWindowChanged = false;
	}

	// draw input window

	if( gInputWindowChanged )
	{
		wclear( gInputWindow );
		wmove( gInputWindow, 0, 0 );

		for( string :: iterator i = gInputBuffer.begin( ); i != gInputBuffer.end( ); i++ )
			waddch( gInputWindow, *i );

		wrefresh( gInputWindow );
		gInputWindowChanged = false;
	}

	// draw channel window

	if( gLobbyWindowChanged )
	{
		wclear( gLobbyWindow );
		mvwaddnstr( gLobbyWindow, 0, gLobbyName.size( ) < 16 ? ( 16 - gLobbyName.size( ) ) / 2 : 0, gLobbyName.c_str( ), 16 );
		mvwhline( gLobbyWindow, 1, 0, 0, 16 );
		int y = 2;

		for( vector<string> :: iterator i = gLobbyPeerNames.begin( ); i != gLobbyPeerNames.end( ); i++ )
		{
			mvwaddnstr( gLobbyWindow, y, 0,  i->c_str(), 16 );
			y++;

			if( y >= LINES - 3 )
				break;
		}

		wrefresh( gLobbyWindow );
		gLobbyWindowChanged = false;
	}
}


void CONSOLE_PrintNoCRLF( string message)
{
	gMainBuffer.push_back( message );

	if( gMainBuffer.size( ) > 100 )
		gMainBuffer.erase( gMainBuffer.begin( ) );

	gMainWindowChanged = true;
	CONSOLE_Draw( );

	if( !gCurses )
		cout << message;
}

void CONSOLE_Print( string message)
{
	CONSOLE_PrintNoCRLF( message );

	if( !gCurses )
		cout << endl;
}



void CONSOLE_Resize( )
{
	if( !gCurses )
		return;

	wresize( gMainWindow, LINES - 3, COLS - 17 );
	wresize( gBottomBorder, 1, COLS );
	wresize( gRightBorder, LINES - 3, 1 );
	wresize( gInputWindow, 2, COLS );
	wresize( gLobbyWindow, LINES - 3, 16 );
	// mvwin( gMainWindow, 0, 0 );
	mvwin( gBottomBorder, LINES - 3, 0 );
	mvwin( gRightBorder, 0, COLS - 17 );
	mvwin( gInputWindow, LINES - 2, 0 );
	mvwin( gLobbyWindow, 0, COLS - 16 );
	mvwhline( gBottomBorder, 0, 0, 0, COLS );
	mvwvline( gRightBorder, 0, 0, 0, LINES );
	wrefresh( gBottomBorder );
	wrefresh( gRightBorder );
	gMainWindowChanged = true;
	gInputWindowChanged = true;
	gLobbyWindowChanged = true;
	CONSOLE_Draw( );
}

bool MindTrisClient :: ServerStatusUpdate(fd_set * fd){

	if( !m_Socket )
		return true;

	m_Socket->DoRecv( fd );
	string * buffer = m_Socket->GetBytes( );
	BYTEARRAY Bytes(buffer->begin(),buffer->end());

	while( Bytes.size() >= DGMTProtocol::HEADERLENGTH )
	{
		DGMTPacket * packet = NULL;
		if(m_Protocol->ExtractPacket(Bytes, &packet))
		{
			if(packet!=NULL)
			{
				uint16_t Length = packet->GetLength();
				m_Packets.push( packet );
				Bytes = BYTEARRAY(Bytes.begin()+Length,Bytes.end());
				*buffer = buffer->substr( Length );
			}else
				break;
		}else{
			m_Error = true;
			m_ErrorString = "received invalid packet from server";
			break;
		}
	}

	while( !m_Packets.empty( ) )
	{
		DGMTPacket *packet = m_Packets.front( );

		m_Packets.pop( );

		switch( packet->GetID( ) )
		{
		case DGMTProtocol::DGMT_LOBBYCREATION:
				{

					DGMTLobbyCreation * info = m_Protocol-> RECEIVE_DGMT_LOBBYCREATION(packet->GetData());
					if(info == NULL) {PrintMalformedPacket(); break;}
					switch(info->GetAnswer())
					{
						case DGMTProtocol::LOBBYCREATION_SUCCESS:
							{
								CONSOLE_Print("[MindTris++] Lobby with ID"+UTIL_ToString(info->GetLobbyID())+" and Session ID "+UTIL_ToString(info->GetSessionID())+ " created.");
								break;
							}
						default :
							{
								CONSOLE_Print("[MindTris++] Error during lobby creation.");
							}
					}
					delete info;
					break;
				}
			case DGMTProtocol::DGMT_LOBBYLIST:
				{
					vector<DGMTLobbyInfo> * v= m_Protocol-> RECEIVE_DGMT_LOBBYLIST(packet->GetData( ));
					if(v == NULL) {PrintMalformedPacket(); break;}
					CONSOLE_Print(" ");
					CONSOLE_Print("  Received Lobby List from Server:");
					CONSOLE_Print(" (* = password protected ) ");
					for (vector<DGMTLobbyInfo>::iterator it = v->begin(); it != v->end(); it++) 
					{
						string pass = ""; if (it->GetHasPassword()) pass = "*";
						CONSOLE_Print("  "+pass+"ID: "+UTIL_ToString(it->GetLobbyId())+"; ["+it->GetLobbyName()+"] created by "+ it->GetCreatorDisplayName()+" ("+UTIL_ToString(it->GetPlayerCount())+"/"+UTIL_ToString(it->GetMaxPlayers())+")");
					}
					delete v;
					break;
				}
			case DGMTProtocol::DGMT_JOINEDLOBBY:
				{
					DGMTJoinedLobby * info = m_Protocol->RECEIVE_DGMT_JOINEDLOBBY(packet->GetData());
					if(info == NULL) {PrintMalformedPacket(); break;}
					switch(info->GetAnswer())
					{
						case DGMTProtocol::JOINEDLOBBY_SUCCESS:
						{
							SetInALobby(true);
							StartP2PSocket();
							m_lobbyid = info->GetLobbyID();
							m_sessionid = info->GetSessionID();
							m_peerid = info->GetPeerID();
							GetPeerInfoVector()->resize(255,NULL);
							CONSOLE_ChangeLobby("Lobby "+UTIL_ToString(m_lobbyid));
							CONSOLE_AddLobbyPeer(GetDisplayName());
							for (vector<DGMTClientLobbyInfo>::iterator it = info->GetClientLobbyList()->begin(); it != info->GetClientLobbyList()->end(); it++) 
							{
								GetPeerInfoVector()->at(it->GetPeerID())= new PeerInfo(it->GetPeerID(), it->GetDisplayName(), it->GetIPAddress(), it->GetPortNumber(), it->GetPublicKey());
								GetPeers()->push_back( new Peer( this, m_P2PProtocol, it->GetPeerID(), it->GetDisplayName(), it->GetIPAddress(), it->GetPortNumber(), it->GetPublicKey()) );
								CONSOLE_AddLobbyPeer(it->GetDisplayName());
							}
							CONSOLE_Print("[MindTris++] Successfully joined lobby ID "+UTIL_ToString(info->GetLobbyID())+".");
							break;
						}
						case DGMTProtocol::JOINEDLOBBY_LOBBYFULL:
						{
							CONSOLE_Print("[MindTris++] Can't join lobby ID "+UTIL_ToString(info->GetLobbyID())+": Lobby is full.");
							break;
						}
						case DGMTProtocol::JOINEDLOBBY_WRONGPASSWORD:
						{
							CONSOLE_Print("[MindTris++] Error while joining lobby ID "+UTIL_ToString(info->GetLobbyID())+": Incorrect password.");
							break;
						}
						default:
						{
							CONSOLE_Print("[MindTris++] Error while joining lobby ID "+UTIL_ToString(info->GetLobbyID())+".");
							break;
						}
					}
					break;
					delete info;
				}
			case DGMTProtocol::DGMT_UPDATECLIENTSTATUS:
				{
					DGMTUpdateClientStatus * info= m_Protocol-> RECEIVE_DGMT_UPDATECLIENTSTATUS(packet->GetData( ));
					if(info == NULL) {PrintMalformedPacket(); break;}
					DGMTClientLobbyInfo * peerinfo = info->GetClientLobbyInfo();
					switch(info->GetStatusUpdate())
					{
						case DGMTProtocol::STATUSUPDATE_HASJOINEDTHELOBBY:
						{
							GetPeerInfoVector()->at(peerinfo->GetPeerID()) = new PeerInfo(peerinfo->GetPeerID(), peerinfo->GetDisplayName(), peerinfo->GetIPAddress(), peerinfo->GetPortNumber(), peerinfo->GetPublicKey());

							CONSOLE_Print("[Mindtris++] User "+peerinfo->GetDisplayName()+" has joined the lobby.");
						
							CONSOLE_AddLobbyPeer(peerinfo->GetDisplayName());
							break;
						}
						case DGMTProtocol::STATUSUPDATE_HASBEENKICKED:
						{
							if(peerinfo->GetPeerID()==GetPeerID())
							{
								UpdateLeaveLobby();
							}else{
								PeerInfo * peerinfop = GetPeerInfoVector()->at(peerinfo->GetPeerID());
								CONSOLE_Print("[Mindtris++] User "+peerinfop->GetDisplayName()+" has been kicked the lobby.");
								CONSOLE_RemoveLobbyPeer(peerinfop->GetDisplayName());
								GetPeerInfoVector()->at(peerinfo->GetPeerID())=NULL;
								delete peerinfop;
								break;
							}
						}
						case DGMTProtocol::STATUSUPDATE_HASLEFTTHELOBBY:
						{
							if(peerinfo->GetPeerID()==GetPeerID())
							{
								UpdateLeaveLobby();
							}else{
								PeerInfo * peerinfop = GetPeerInfoVector()->at(peerinfo->GetPeerID());
								CONSOLE_Print("[Mindtris++] User "+peerinfop->GetDisplayName()+" has left the lobby.");
								CONSOLE_RemoveLobbyPeer(peerinfop->GetDisplayName());
								GetPeerInfoVector()->at(peerinfo->GetPeerID())=NULL;
								delete peerinfop;
								break;
							}
						}
					}
				}
			default:
				 break;
		}

		delete packet;
	}
	if(m_Error)
	{
		CONSOLE_Print( "[MindTris++] Disconnected from server: " + m_ErrorString );
		m_DeleteMe = true;
	}
	m_DeleteMe = m_DeleteMe || m_Socket->HasError( ) || !m_Socket->GetConnected( );
	return m_DeleteMe;
}

bool MindTrisClient ::Update(long usecBlock){
	unsigned int NumFDs = 0;

	int nfds = 0;
	fd_set fd;
	fd_set send_fd;
	FD_ZERO( &fd );
	FD_ZERO( &send_fd );

	// Peer sockets

	for( vector<Peer *> :: iterator i = GetPeers()->begin( ); i != GetPeers()->end( ); i++ )
	{
		(*i)->GetSocket( )->SetFD( &fd, &send_fd, &nfds );
		NumFDs++;
	}

	// Server Socket

	
	if(m_Socket){
		m_Socket->SetFD( &fd, &send_fd, &nfds );
		NumFDs++;
	}


	// P2P Listening Socket

	if(m_P2PSocket){
		m_P2PSocket->SetFD( &fd, &send_fd, &nfds );
		NumFDs++;
	}

	if( usecBlock < 1000 )
		usecBlock = 1000;

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = usecBlock;

	struct timeval send_tv;
	send_tv.tv_sec = 0;
	send_tv.tv_usec = 0;

	#ifdef WIN32
		select( 1, &fd, NULL, NULL, &tv );
		select( 1, NULL, &send_fd, NULL, &send_tv );
	#else
		select( nfds + 1, &fd, NULL, NULL, &tv );
		select( nfds + 1, NULL, &send_fd, NULL, &send_tv );
	#endif

	if( NumFDs == 0 )
	{
		MILLISLEEP( 50 );
	}

	if( m_Socket)
	{
		ServerStatusUpdate(&fd);

		m_Socket->DoSend(&send_fd);
	}

	if( m_P2PSocket )
	{
		CTCPSocket *NewSocket = m_P2PSocket->Accept( &fd );

		if( NewSocket )
		{

				NewSocket->SetNoDelay( true );

				GetPeers()->push_back( new Peer( this, m_P2PProtocol, NewSocket ) );

				CONSOLE_Print( "[MindTris++] connection attempt from [" + NewSocket->GetIPString( ) + "]" );
		}

		if( m_P2PSocket->HasError( ) )
		{
			CONSOLE_Print( "[MindTris++] Socket error: " + m_P2PSocket->GetErrorString()  );
			return true;
		}
	}

	for( vector<Peer *> :: iterator i = GetPeers()->begin( ); i != GetPeers()->end( );  )
	{

		if( (*i)->Update( &fd ) )
		{
			delete *i;
			i = GetPeers()->erase( i );
		}
		else i++;
	}


	for( vector<Peer *> :: iterator i = GetPeers()->begin( ); i != GetPeers()->end( ); i++ )
	{
			if( (*i)->GetSocket( ) )
			{
				if( (*i)->GetSocket()->GetConnected()) (*i)->GetSocket( )->DoSend( &send_fd );
				else{
					if((*i)->GetClientInitiatedHandshake()){
						if(  (*i)->GetClientSocket( )->GetConnecting( ) )
						{
						// we are currently attempting to connect

						(*i)->GetClientSocket( )->CheckConnect( );
						}
					}
				}
				
			}
	}
	return false;

}

void MindTrisClient ::SendChatCommand(string message){
	for( vector<Peer *> :: iterator i = GetPeers()->begin( ); i != GetPeers()->end( );  i++ )
	{
		(*i)->GetSocket()->PutBytes(m_P2PProtocol->SEND_CHATSEND("", message));
	}

}

void MindTrisClient :: PrintMalformedPacket(){
	CONSOLE_Print( "[MindTris++] Received malformed packet from [" + m_Socket->GetIPString( ) + "]" );
}

void MindTrisClient::RegisterUser(string username, string displayname, string email, string password){
	CONSOLE_Print("[MindTris++] Registering new user...");
	fd_set fd;
	fd_set send_fd;
	int nfds = 0;

	m_Socket->PutBytes(m_Protocol->SEND_DGMT_CREATEUSER(username,displayname,email,password,m_Encryptor));


	bool done = false;
	while(!done)
	{
	
		FD_ZERO( &fd );
		FD_ZERO( &send_fd );
		m_Socket->SetFD( &fd, &send_fd, &nfds );

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 50000;

		struct timeval send_tv;
		send_tv.tv_sec = 0;
		send_tv.tv_usec = 0;

		#ifdef WIN32
			select( 1, &fd, NULL, NULL, &tv );
			select( 1, NULL, &send_fd, NULL, &send_tv );
		#else
			select( nfds + 1, &fd, NULL, NULL, &tv );
			select( nfds + 1, NULL, &send_fd, NULL, &send_tv );
		#endif

		m_Socket->DoSend(&send_fd);
		m_Socket->DoRecv(&fd);

		string * buffer = m_Socket->GetBytes( );

		while( buffer->size( ) >=  DGMTProtocol::HEADERLENGTH )
		{
			BYTEARRAY Bytes(buffer->begin(),buffer->end());
			DGMTPacket * packet = NULL;
			if(m_Protocol->ExtractPacket(Bytes, &packet))
			{
				if(packet!=NULL)
				{
					uint16_t Length = packet->GetLength();
					DGMTProtocol::UserCreation reply;
					if(packet->GetID() == DGMTProtocol::DGMT_USERCREATION)
					{
						reply = m_Protocol->RECEIVE_DGMT_USERCREATION(packet->GetData());
						switch(reply)
						{
							case DGMTProtocol::USERCREATION_SUCCESS:
								{
									CONSOLE_Print("[MindTris++] Successfully registered new user."); break;
								}
							default:
								{
									CONSOLE_Print("[MindTris++] Error during registration."); break;
								}
						}
						done = true;
					}


					Bytes = BYTEARRAY(Bytes.begin()+Length,Bytes.end());
					*buffer = buffer->substr( Length );
				}else{
					break;
				}
			}else{
				m_Error = true;
				break;
			}
		}
		if(m_Error)
		{
			CONSOLE_Print("[MindTris++] received invalid packet from server. Closing...");
			this->~MindTrisClient();
			return;
		}
	}

}

void MindTrisClient::Login(string username, string password)
{
	CONSOLE_Print("[MindTris++] Logging in user "+username+"...");
	
	fd_set fd;
	fd_set send_fd;
	int nfds = 0;
	m_Socket->PutBytes(m_Protocol->SEND_DGMT_LOGIN(username,password,m_Encryptor));

	bool done = false;
	while(!done)
	{
	
		FD_ZERO( &fd );
		FD_ZERO( &send_fd );
		m_Socket->SetFD( &fd, &send_fd, &nfds );

	
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 50000;

		struct timeval send_tv;
		send_tv.tv_sec = 0;
		send_tv.tv_usec = 0;

		#ifdef WIN32
			select( 1, &fd, NULL, NULL, &tv );
			select( 1, NULL, &send_fd, NULL, &send_tv );
		#else
			select( nfds + 1, &fd, NULL, NULL, &tv );
			select( nfds + 1, NULL, &send_fd, NULL, &send_tv );
		#endif

		m_Socket->DoSend(&send_fd);
		m_Socket->DoRecv(&fd);

		string * buffer = m_Socket->GetBytes( );

		while( buffer->size( ) >=  DGMTProtocol::HEADERLENGTH )
		{
			BYTEARRAY Bytes(buffer->begin(),buffer->end());
			DGMTPacket * packet = NULL;
			if(m_Protocol->ExtractPacket(Bytes, &packet))
			{
				if(packet!=NULL)
				{
					uint16_t Length = packet->GetLength();
					DGMTLoginReply * reply;
					if(packet->GetID() == DGMTProtocol::DGMT_LOGINREPLY)
					{
						reply = m_Protocol->RECEIVE_DGMT_LOGINREPLY(packet->GetData());
						if(reply ==NULL) 
						{
							PrintMalformedPacket();
						}else{
							switch(reply->GetAnswer())
							{
								case DGMTProtocol::LOGINREPLY_SUCCESS:
								case DGMTProtocol::LOGINREPLY_SUCCESSDISCONNECTEDELSEWHERE:
									{
										CONSOLE_Print("[MindTris++] Successfully logged in!");
										SetHasLoggedIn(true);
										SetDisplayName(reply->GetDisplayName());
										break;
									}
								case DGMTProtocol::LOGINREPLY_BADUSERNAMEPASSWORD:
									{
										CONSOLE_Print("[MindTris++] Password/Username mismatch."); break;
									}
								case DGMTProtocol::LOGINREPLY_USERNAMEDOESNOTEXIST:
									{
										CONSOLE_Print("[MindTris++] Username does not exist."); break;
									}
								default:
									{
										CONSOLE_Print("[MindTris++] Unknown error while signing in."); break;
									}
							}
							done = true;
						}
					}

					Bytes = BYTEARRAY(Bytes.begin()+Length,Bytes.end());
					*buffer = buffer->substr( Length );
				}else
					break;
			}else{
				m_Error = true;
				break;
			}
		}
		if(m_Error)
		{
			CONSOLE_Print("[MindTris++] received invalid packet from server. Closing...");
			this->~MindTrisClient();
			return;
		}
	}

}

void MindTrisClient::CreateLobby(string lobbyname, uint8_t maxplayers,bool haspassword,string password){
	m_Socket->PutBytes(m_Protocol->SEND_DGMT_CREATELOBBY(lobbyname,maxplayers,haspassword,password,m_Encryptor));
}

void MindTrisClient::JoinLobby(uint32_t lobbyid, string password){
	m_Socket->PutBytes(m_Protocol->SEND_DGMT_JOINLOBBY(lobbyid,password,m_listenport,m_PublicKey));
}

void MindTrisClient::LeaveLobby(){
	m_Socket->PutBytes(m_Protocol->SEND_DGMT_LEAVELOBBY());
}

void MindTrisClient::RetrieveLobbyList(){
	m_Socket->PutBytes(m_Protocol->SEND_DGMT_GETLOBBYLIST());
}

void MindTrisClient::StopP2PSocket(){
	if(m_P2PSocket){
		delete m_P2PSocket;
		m_P2PSocket = NULL;
	}
}

void MindTrisClient::StartP2PSocket(){
	m_P2PSocket = new CTCPServer(&CONSOLE_Print,true);
	if( m_P2PSocket->Listen( "", m_listenport ) )
		CONSOLE_Print( "[MindTris++] listening on port " + UTIL_ToString( m_listenport ) );
	else
	{
		CONSOLE_Print( "[MindTris++] error listening on port " + UTIL_ToString( m_listenport ) );
	}
}

MindTrisClient::MindTrisClient(string address, uint16_t port, uint16_t clientport){

	CryptoPP::AutoSeededRandomPool rng;
	CryptoPP::InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(rng, 3072);

	CryptoPP::Integer modulus = params.GetModulus();
	CryptoPP::Integer exponent = params.GetPublicExponent();
		
	string modulusstring;
	CryptoPP::TransparentFilter modulusFilter(new CryptoPP::StringSink(modulusstring));
	modulus.Encode(modulusFilter,modulus.MinEncodedSize());

	string exponentstring;
	CryptoPP::TransparentFilter exponentFilter(new CryptoPP::StringSink(exponentstring));
	exponent.Encode(exponentFilter,exponent.MinEncodedSize());

	m_PublicKey = new RSAPublicKey();
	m_PublicKey->Exponent = exponentstring;
	m_PublicKey->Modulus = modulusstring;

	m_peers = new vector<Peer *>;
	m_peerinfovector = new vector<PeerInfo *>;
	m_DeleteMe = false;
	m_P2PProtocol = new DGMTP2PProtocol(true);
	m_P2PSocket = NULL;
	m_listenport = clientport;
	m_inalobby = false;
	m_Socket = new CTCPClient( &CONSOLE_Print, false);
	m_Error= false;
	m_hasloggedin = false;
	m_Protocol = new DGMTProtocol(true);

	CONSOLE_Print("[MindTris++] Attempting to connect to server at ["+address+"] on port "+ UTIL_ToString(port));
	CONSOLE_Print("[MindTris++] Connecting...");

	m_Socket-> Connect( "", address, port );

	while(!m_Socket->CheckConnect())
	{
		MILLISLEEP(100);
	}

	CONSOLE_Print("[MindTris++] Checking Server Version...");

	m_Socket->PutBytes(m_Protocol->SEND_DGMT_HELLOFROMCLIENT());
	
	CONSOLE_Print("[MindTris++] Awaiting reply...");

	fd_set fd;
	fd_set send_fd;
	int nfds = 0;

	bool done = false;
	while(!done)
	{
	
		FD_ZERO( &fd );
		FD_ZERO( &send_fd );
		m_Socket->SetFD( &fd, &send_fd, &nfds );

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 50000;

		struct timeval send_tv;
		send_tv.tv_sec = 0;
		send_tv.tv_usec = 0;

		#ifdef WIN32
			select( 1, &fd, NULL, NULL, &tv );
			select( 1, NULL, &send_fd, NULL, &send_tv );
		#else
			select( nfds + 1, &fd, NULL, NULL, &tv );
			select( nfds + 1, NULL, &send_fd, NULL, &send_tv );
		#endif



		m_Socket->DoSend(&send_fd);
		m_Socket->DoRecv(&fd);

		string * buffer = m_Socket->GetBytes( );

		while( buffer->size( ) >=  DGMTProtocol::HEADERLENGTH )
		{
			BYTEARRAY Bytes(buffer->begin(),buffer->end());

			DGMTPacket * packet = NULL;
			if(m_Protocol->ExtractPacket(Bytes, &packet))
			{
				if(packet!=NULL)
				{
					uint16_t Length = packet->GetLength();
					DGMTConnectionReply * reply;
					if(packet->GetID() == DGMTProtocol::DGMT_HELLOFROMSERVER)
					{
						reply = m_Protocol->RECEIVE_DGMT_HELLOFROMSERVER(packet->GetData());
						if(reply==NULL){ 
							CONSOLE_Print("[MindTris++] received malformed HELLO_FROM_SERVER packet from server. Closing...");
							assert(false);
						}else{
							if(reply->Connected()){ 
								CONSOLE_Print("[MindTris++] Successfully connected to Server. Message from server:");
								m_serverPublicKey = reply->GetPublicKey();
								string modulusstring = m_serverPublicKey->Modulus;
								string exponentstring = m_serverPublicKey->Exponent;
								CryptoPP::Integer modulus;
								CryptoPP::Integer exponent;

								modulus.Decode(CryptoPP::StringSource(modulusstring,true), modulusstring.size());
								exponent.Decode(CryptoPP::StringSource(exponentstring,true), exponentstring.size());
								CryptoPP::RSAFunction f;
								f.Initialize(modulus,exponent);
								m_Encryptor = new CryptoPP::RSAES_OAEP_SHA_Encryptor(f);
								CONSOLE_Print(reply->GetMessage());
							}else{
								CONSOLE_Print("[MindTris++] Message from server:" + reply->GetMessage());
							}
							delete reply;
						}
						done = true;
					}
	
					Bytes = BYTEARRAY(Bytes.begin()+Length,Bytes.end());
					*buffer = buffer->substr( Length );
				}else
					break;
			}else{
				m_Error = true;
				break;
			}
		}
		if(m_Error)
		{
			CONSOLE_Print("[MindTris++] received invalid packet from server. Closing...");
			delete m_Socket;
			return;
		}
	}

}

void WaitForStringInput(string question, string helper, string &s){
	CONSOLE_Print( "");
	CONSOLE_Print( "  "+question+" ");
	CONSOLE_Print( "");

	do
	{
		CONSOLE_PrintNoCRLF( "  "+helper+": " );
		getline( cin, s );
	} while( s.empty() );
}

void MindTrisClient::UpdateLeaveLobby()
{
	CONSOLE_Print("[MindTris++] Leaving lobby. Closing peer connections ...");
	for (vector<PeerInfo *>::iterator it = GetPeerInfoVector()->begin(); it != GetPeerInfoVector()->end(); it++) 
	{
		if( *it !=NULL)
		{
			delete (*it);
		}
	}
	GetPeerInfoVector()->clear();
	for (vector<Peer *>::iterator it = GetPeers()->begin(); it != GetPeers()->end(); it++) 
	{
		CONSOLE_RemoveLobbyPeer((*it)->GetDisplayName());
		delete (*it);
	}
	GetPeers()->clear();
	CONSOLE_ChangeLobby("");
	CONSOLE_RemoveLobbyPeer(GetDisplayName());
	StopP2PSocket();
	SetInALobby(false);
	CONSOLE_Print("[MindTris++] Successfully left the lobby.");



}

enum TextCommandMode
{
	TEXTCOMMANDMODE_GENERAL,
	TEXTCOMMANDMODE_LOBBYCHATTING,
	TEXTCOMMANDMODE_CREATELOBBYCOMMAND_LOBBYNAME,
	TEXTCOMMANDMODE_CREATELOBBYCOMMAND_MAXPLAYERS,
	TEXTCOMMANDMODE_CREATELOBBYCOMMAND_PASSWORD,
	TEXTCOMMANDMODE_JOINLOBBYCOMMAND_LOBBYID,
	TEXTCOMMANDMODE_JOINLOBBYCOMMAND_PASSWORD,
};

int main(int argc, char ** argv)
{
#ifdef WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

	setlocale(LC_ALL,"");

	/*int bufferSize = WideCharToMultiByte(CP_UTF8, 0, s, -1, NULL, 0, NULL, NULL);

	char* m = new char[bufferSize]; 

	WideCharToMultiByte(CP_UTF8, 0, s, -1, m,bufferSize, NULL, NULL);  */

	#ifdef WIN32
		// initialize winsock

		CONSOLE_Print( "[MindTris++] starting winsock" );
		WSADATA wsadata;

		if( WSAStartup( MAKEWORD( 2, 2 ), &wsadata ) != 0 )
		{
			CONSOLE_Print( "[MindTris++] error starting winsock" );
			return 1;
		}
	#endif

	string server_address = "";
	uint16_t server_port = 0;
	uint16_t client_port = 0;

 	switch(argc)
	{
		case 2:
			server_address = argv[1];
			break;
		case 3:
			server_address = argv[1]; server_port = (uint16_t) atoi(argv[2]);
			break;
		case 4:
			server_address = argv[1]; server_port = (uint16_t) atoi(argv[2]); client_port = (uint16_t) atoi(argv[3]);
			break;
	}

	CONSOLE_Print( "");
	CONSOLE_Print( " To use the MindTris++ client, you need to connect to a MindTris Server.");
	CONSOLE_Print( "");
	if( server_address.empty()){
		WaitForStringInput("Enter the server's ip address or hostname.", "Hostname/IP",server_address);
	}

	if(server_port== 0){
		CONSOLE_Print( "");
		CONSOLE_Print( "  Enter the server's listening port." );
		CONSOLE_Print( "");
		string server_portS;
		do
		{
			CONSOLE_PrintNoCRLF( "  ServerPort: " );
			getline( cin, server_portS );
			server_port = atoi(server_portS.c_str());
		} while( server_port==0 );
	}

	if(client_port== 0){
		CONSOLE_Print( "");
		CONSOLE_Print( "  Enter the client's listening port (TCP)." );
		CONSOLE_Print( "  This port will be used for P2P communications and should be open on the client side." );
		CONSOLE_Print( "");
		string client_portS;
		do
		{
			CONSOLE_PrintNoCRLF( "  Client Port: " );
			getline( cin, client_portS );
			client_port = atoi(client_portS.c_str());
		} while( client_port==0 );
	}


	MindTrisClient * client = new MindTrisClient(server_address, server_port, client_port);

	while(!client->HasLoggedIn())
	{
		CONSOLE_Print( "");
		CONSOLE_Print( "  Please, either register a new user or login with an existing one." );
		CONSOLE_Print( "  What do you want to do?" );
		CONSOLE_Print( "	1. Register a new user.");
		CONSOLE_Print( "	2. Login with an existing user.");
		CONSOLE_Print( "");

		string input; int inputID;

		do
		{
			CONSOLE_PrintNoCRLF( "  Option #: " );
			getline( cin, input );
			inputID = atoi(input.c_str());
		} while( inputID!=1 && inputID!=2);

		switch(inputID)
		{
			case 1:
				{
					string username; string displayname; string email; string password;
					WaitForStringInput("What username do you want to register?", "Username",username);
					WaitForStringInput("What display name do you want to use?", "Display Name",displayname);
					WaitForStringInput("What is your email address?", "Email",email);
					WaitForStringInput("Please type in a password (it will be visible).", "Password",password);
					client->RegisterUser(username,displayname,email,password);
					break;
				}
			case 2:
				{
					string username; string password;
					WaitForStringInput("Which username to want to log as?", "Username",username);
					WaitForStringInput("Type in your password (it will be visible).", "Password",password);
					client->Login(username,password);
					break;
				}
		}
	}


	
	// initialize curses

		gCurses = true;
		initscr( );
	#ifdef WIN32
		resize_term( 28, 97 );
	#endif

		clear( );
		noecho( );
		cbreak( );
		gMainWindow = newwin( LINES - 3, COLS - 17, 0, 0 );
		gBottomBorder = newwin( 1, COLS, LINES - 3, 0 );
		gRightBorder = newwin( LINES - 3, 1, 0, COLS - 17 );
		gInputWindow = newwin( 2, COLS, LINES - 2, 0 );
		gLobbyWindow = newwin( LINES - 3, 16, 0, COLS - 16 );
		mvwhline( gBottomBorder, 0, 0, 0, COLS );
		mvwvline( gRightBorder, 0, 0, 0, LINES );
		wrefresh( gBottomBorder );
		wrefresh( gRightBorder );
		scrollok( gMainWindow, TRUE );
		keypad( gInputWindow, TRUE );
		scrollok( gInputWindow, TRUE );

		CONSOLE_Print( "  Type /help at any time for help." );
		CONSOLE_Print( "  Press any key to continue." );
		CONSOLE_Print( "" );

		CONSOLE_Draw( );
		wgetch( gInputWindow );
		nodelay( gInputWindow, TRUE );

	TextCommandMode commandMode = TEXTCOMMANDMODE_GENERAL;


	string createlobby_lobbyname;
	uint8_t createlobby_maxplayers;
	string createlobby_password;

	uint32_t joinlobby_lobbyid;
	string joinlobby_password;

	while( 1 )
	{
		if( client->Update( 40000 ) )
			break;
		if(client->InALobby()) commandMode = TEXTCOMMANDMODE_LOBBYCHATTING;
		if(commandMode == TEXTCOMMANDMODE_LOBBYCHATTING && !client->InALobby()) commandMode = TEXTCOMMANDMODE_GENERAL;
		bool Quit = false;
		int c = wgetch( gInputWindow );

		while( c != ERR )
		{
			if( c == 8 || c == 127 || c == KEY_BACKSPACE || c == KEY_DC )
			{
				// backspace, delete

				if( !gInputBuffer.empty( ) )
					gInputBuffer.erase( gInputBuffer.size( ) - 1, 1 );
			}
			else if( c == 9 )
			{
				// tab
			}
#ifdef WIN32
			else if( c == 10 || c == 13 || c == PADENTER )
#else
			else if( c == 10 || c == 13 )
#endif
			{
			switch(commandMode)
			{
				case TEXTCOMMANDMODE_GENERAL:
					{
						string Command = gInputBuffer;
						transform( Command.begin( ), Command.end( ), Command.begin( ), (int(*)(int))tolower );

						if( Command == "/commands" )
						{
							CONSOLE_Print( ">>> /commands" );
							CONSOLE_Print( "" );
							CONSOLE_Print( "  In the MindTris++ console:" );
							CONSOLE_Print( "   /commands           : show command list");
							CONSOLE_Print( "   /createlobby        : create a lobby");
							CONSOLE_Print( "   /lobbylist          : display list of lobbies");
							CONSOLE_Print( "   /joinlobby          : join a lobby");
							CONSOLE_Print( "   /exit or /quit      : close MindTris++" );
							CONSOLE_Print( "   /version            : show version text" );
							CONSOLE_Print( "  In a lobby:" );
							CONSOLE_Print( "   /leavelobby       : leave the lobby");
							CONSOLE_Print( "" );
						}
						else if( Command == "/lobbylist")
						{
							CONSOLE_PrintNoCRLF( "  Retrieving Lobby List:");
							client->RetrieveLobbyList();
						}
						else if( Command == "/createlobby")
						{
							commandMode =  TEXTCOMMANDMODE_CREATELOBBYCOMMAND_LOBBYNAME;
							CONSOLE_Print( "" );
							CONSOLE_Print( "  Please enter the name of the lobby you want to create: ");
							CONSOLE_Print( "" );
							CONSOLE_PrintNoCRLF( "  Lobby Name:");
						}
						else if( Command == "/joinlobby")
						{
							commandMode =  TEXTCOMMANDMODE_JOINLOBBYCOMMAND_LOBBYID;
							CONSOLE_Print( "" );
							CONSOLE_Print( "  Please enter the lobby id of the lobby you want to join: ");
							CONSOLE_Print( "" );
							CONSOLE_PrintNoCRLF( "  Lobby ID:");
						}
						else if( Command == "/exit" || Command == "/quit" )
						{
							Quit = true;
							break;
						}
						else if( Command == "/help" )
						{
							CONSOLE_Print( ">>> /help" );
							CONSOLE_Print( "" );
							CONSOLE_Print( "  MindTris++ connects to a MindTris Server." );
							CONSOLE_Print( "  Once it connects, you can create a user or login with an existing one." );
							CONSOLE_Print( "  Once you login, you can either create a lobby or join one." );
							CONSOLE_Print( "" );
							CONSOLE_Print( "  If you want to create a lobby, type /createlobby" );
							CONSOLE_Print( "  To join a lobby, type /joinlobby" );
							CONSOLE_Print( "" );
							CONSOLE_Print( "  Type \"/commands\" for a full command list." );
							CONSOLE_Print( "" );
						}
						else if( Command == "/version" )
							CONSOLE_Print( "[MindTris++] MindTris++ Version " + client->GetVersionS() );
						break;
					}
				case TEXTCOMMANDMODE_CREATELOBBYCOMMAND_LOBBYNAME:
					{
						createlobby_lobbyname= gInputBuffer;
						if(!createlobby_lobbyname.empty()){
							commandMode = TEXTCOMMANDMODE_CREATELOBBYCOMMAND_MAXPLAYERS;
							CONSOLE_Print( "" );
							CONSOLE_Print( "  Please enter the maximum number of players allowed (1-255). ");
							CONSOLE_Print( "" );
							CONSOLE_PrintNoCRLF( "  Max Players:");
						}
						break;
					}
				case TEXTCOMMANDMODE_CREATELOBBYCOMMAND_MAXPLAYERS:
					{
						createlobby_maxplayers= (uint8_t) atoi(gInputBuffer.c_str());
						if(createlobby_maxplayers >0){
							commandMode =  TEXTCOMMANDMODE_CREATELOBBYCOMMAND_PASSWORD;
							CONSOLE_Print( "" );
							CONSOLE_Print( "  Please enter a lobby password (empty password for no password): ");
							CONSOLE_Print( "" );
							CONSOLE_PrintNoCRLF( "  Password:");
						}
						break;
					}
				case TEXTCOMMANDMODE_CREATELOBBYCOMMAND_PASSWORD:
					{
						createlobby_password = gInputBuffer;
						client->CreateLobby(createlobby_lobbyname,createlobby_maxplayers,!createlobby_password.empty(),createlobby_password);
						CONSOLE_Print( "  Create lobby "+createlobby_lobbyname+"...");
						commandMode = TEXTCOMMANDMODE_GENERAL;
						break;
					}
				case TEXTCOMMANDMODE_JOINLOBBYCOMMAND_LOBBYID:
					{
						joinlobby_lobbyid=  (uint32_t) atoi(gInputBuffer.c_str());
						commandMode = TEXTCOMMANDMODE_JOINLOBBYCOMMAND_PASSWORD;
						CONSOLE_Print( "" );
						CONSOLE_Print( "  Please enter the password to the lobby: ");
						CONSOLE_Print( "" );
						CONSOLE_PrintNoCRLF( "  Password:");
						break;
					}
				case TEXTCOMMANDMODE_JOINLOBBYCOMMAND_PASSWORD:
					{
						joinlobby_password = gInputBuffer;
						client->JoinLobby(joinlobby_lobbyid,joinlobby_password);
						CONSOLE_Print( "  Attempting to join lobby ID: "+UTIL_ToString(joinlobby_lobbyid)+"...");
						commandMode = TEXTCOMMANDMODE_GENERAL;
						break;
					}
				case TEXTCOMMANDMODE_LOBBYCHATTING:
					{
						
						string Command = gInputBuffer;
						transform( Command.begin( ), Command.end( ), Command.begin( ), (int(*)(int))tolower );

						if( Command == "/leavelobby" )
						{
							client->LeaveLobby();
						}else{
							client->SendChatCommand( gInputBuffer );
							CONSOLE_Print( "[P2P Chat] "+client->GetDisplayName()+": "+gInputBuffer);
						}
						break;
					}
				}					
				gInputBuffer.clear( );
			}
#ifdef WIN32
			else if( c == 22 )
			{
				// paste

				char *clipboard = NULL;
				long length = 0;

				if( PDC_getclipboard( &clipboard, &length ) == PDC_CLIP_SUCCESS )
				{
					gInputBuffer += string( clipboard, length );
					PDC_freeclipboard( clipboard );
				}
			}
#endif
			else if( c == 27 )
			{
				// esc

				gInputBuffer.clear( );
			}
			else if( c >= 32 && c <= 255 )
			{
				// printable characters

				gInputBuffer.push_back( c );
			}
#ifdef WIN32
			else if( c == PADSLASH )
				gInputBuffer.push_back( '/' );
			else if( c == PADSTAR )
				gInputBuffer.push_back( '*' );
			else if( c == PADMINUS )
				gInputBuffer.push_back( '-' );
			else if( c == PADPLUS )
				gInputBuffer.push_back( '+' );
#endif
			else if( c == KEY_RESIZE )
				CONSOLE_Resize( );

			// clamp input buffer size

			if( gInputBuffer.size( ) > 200 )
				gInputBuffer.erase( 200 );

			c = wgetch( gInputWindow );
			gInputWindowChanged = true;
		}

		CONSOLE_Draw( );

		if( Quit )
			break;
	}

	// shutdown gproxy

	CONSOLE_Print( "[MindTris++] shutting down" );
	delete client;
	client = NULL;

#ifdef WIN32
	// shutdown winsock

	CONSOLE_Print( "[MindTris++] shutting down winsock" );
	WSACleanup( );
#endif

	// shutdown curses

	endwin( );
	return 0;

}


MindTrisClient:: ~MindTrisClient()
{
	if( m_Socket )
		delete m_Socket;
	
}