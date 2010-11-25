
#include "mindtriscore/includes.h"
#include "mindtriscore/util.h"
#include "mindtriscore/socket.h"
#include "mindtriscore/oalloc.h"
#include "mindtriscore/serverprotocol.h"
#include "mindtriscore/p2pprotocol.h"
#include "database.h"
#include "mt_server.h"
#include "lobby.h"
#include "user.h"


uint32_t User :: GetIPInt32(){ return m_Socket->GetIPInt32();}

void User :: SendPeerStatusUpdate(User * u, DGMTProtocol::StatusUpdate status)
{
	switch(status)
	{
		case DGMTProtocol::STATUSUPDATE_HASJOINEDTHELOBBY:
			{
				this->Send(m_Protocol->SEND_DGMT_UPDATECLIENTSTATUS(status,u->GetPeerID(),u->GetDisplayName(),u->GetIPInt32(),u->GetP2PPortNumber(),u->GetPublicKey())); break;
			}
		case DGMTProtocol::STATUSUPDATE_HASLEFTTHELOBBY:
			{
				this->Send(m_Protocol->SEND_DGMT_UPDATECLIENTSTATUS(status,u->GetPeerID())); break;
			}
		case DGMTProtocol::STATUSUPDATE_HASBEENKICKED:
			{
				this->Send(m_Protocol->SEND_DGMT_UPDATECLIENTSTATUS(status,u->GetPeerID())); break;
			}
	}
}

bool User :: LeaveLobby(){
	if(this->GetLobby()->UserLeave(this))
	{
		SetLobby( NULL);
		return true;
	}else{
		return false;
	}
}

bool User :: JoinLobby(Lobby * lobby, string password, DGMTProtocol::JoinedLobby * answer){
	if(lobby->UserJoin(this, password, answer))
	{
		SetLobby(lobby); return true;
	}
	return false;
}


void User :: PrintMalformedPacket(){
	CONSOLE_Print( "[MindTris Server] Received malformed packet from [" + m_Socket->GetIPString( ) + "]" );
}

bool User :: Update(fd_set * fd){

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
			m_ErrorString = "received invalid packet from user";
			break;
		}
	}

	while( !m_Packets.empty( ) )
	{
		DGMTPacket *packet = m_Packets.front( );

		m_Packets.pop( );

		switch( packet->GetID( ) )
		{
		case  DGMTProtocol::DGMT_HELLOFROMCLIENT:
				 {
					uint32_t ProtocolVersion = 0;
					ProtocolVersion = m_Protocol-> RECEIVE_DGMT_HELLOFROMCLIENT( packet->GetData( ) );
					if(ProtocolVersion == m_Protocol->DGMT_PROTOCOLVERSION)
					{
						Send(m_Protocol->SEND_DGMT_HELLOFROMSERVER(m_Protocol->SHELLO_CONNECTED,  m_Server->GetPublicKey(), m_Server->GetMOTD() ));
						CONSOLE_Print( "[MindTris Server] connection accepted from [" + m_Socket->GetIPString( ) + "]" );
					}else{
						Send(m_Protocol->SEND_DGMT_HELLOFROMSERVER(m_Protocol->SHELLO_REFUSEDWRONGVERSION,   m_Server->GetPublicKey(), ""));
						CONSOLE_Print( "[MindTris Server] connection refused from [" + m_Socket->GetIPString( ) + "], wrong protocol version" );
						m_DeleteMe = true;
					}
					break;
				 }
			case DGMTProtocol::DGMT_CREATEUSER:
				{

					DGMTCreateUserInfo * info = m_Protocol-> RECEIVE_DGMT_CREATEUSER( packet->GetData( ), m_Server-> GetDecryptor());
					if(info == NULL) {PrintMalformedPacket(); break;}
					if(m_Server->GetDatabase()->UserExists(info->GetUsername()))
					{
						Send(m_Protocol->SEND_DGMT_USERCREATION(m_Protocol->USERCREATION_USERNAMEALREADYEXISTS));
					}else{
						m_Server->GetDatabase()->AddUser(info->GetUsername(),info->GetDisplayName(),info->GetEmail(),info->GetPassword());
						Send(m_Protocol->SEND_DGMT_USERCREATION(m_Protocol->USERCREATION_SUCCESS));
						CONSOLE_Print("[MindTris Server] Created User ["+info->GetUsername()+"].");
					}
					delete info;
					break;
				}
			case DGMTProtocol::DGMT_LOGIN:
				{

					DGMTLoginInfo * info = m_Protocol-> RECEIVE_DGMT_LOGIN(packet->GetData( ), m_Server-> GetDecryptor());
					if(info == NULL) {PrintMalformedPacket(); break;}
					UserData * userdata = NULL;
					if(m_Server->GetDatabase()->UserMatchesPassword(info->GetUsername(),info->GetPassword(), userdata))
					{
						CONSOLE_Print("[MindTris Server] User ["+info->GetUsername()+"] has logged in.");
						SetDisplayName(userdata->GetDisplayName());
						Send(m_Protocol->SEND_DGMT_LOGINREPLY(m_Protocol->LOGINREPLY_SUCCESS,userdata->GetDisplayName()));
					}else{
						if(userdata != NULL)
						{
							CONSOLE_Print("[MindTris Server] Client ["+ m_Socket->GetIPString( )+"] log in: password/username mismatch.");
							Send(m_Protocol->SEND_DGMT_LOGINREPLY(m_Protocol->LOGINREPLY_BADUSERNAMEPASSWORD,""));

						}else{
							CONSOLE_Print("[MindTris Server] Client ["+ m_Socket->GetIPString( )+"] log in: username ["+info->GetUsername()+"] does not exist.");
							Send(m_Protocol->SEND_DGMT_LOGINREPLY(m_Protocol->LOGINREPLY_USERNAMEDOESNOTEXIST,""));
						}
					}
					delete info;
					break;
				}
			case DGMTProtocol::DGMT_CREATELOBBY:
				{

					DGMTCreateLobby * info = m_Protocol-> RECEIVE_DGMT_CREATELOBBY(packet->GetData( ), m_Server-> GetDecryptor());
					if(info == NULL) {PrintMalformedPacket(); break;}
					CONSOLE_Print("[MindTris Server] Player ["+GetDisplayName()+"] trying to create lobby "+info->GetLobbyName()+".");
					Lobby * l =m_Server->CreateLobby(this, info->GetLobbyName(),info->GetMaxPlayers(),info->GetHasPassword(),info->GetPassword());
					Send(m_Protocol->SEND_DGMT_LOBBYCREATION(DGMTProtocol::LOBBYCREATION_SUCCESS,l->GetLobbyID(),l->GetSessionID()));
					delete info;
					break;
				}
			case DGMTProtocol::DGMT_GETLOBBYLIST:
				{
					m_Protocol-> RECEIVE_DGMT_GETLOBBYLIST(packet->GetData( ));
					CONSOLE_Print("[MindTris Server] Player ["+GetDisplayName()+"] trying to retrieve lobby list.");
					vector<DGMTLobbyInfo> * lobbiesinfo = new vector<DGMTLobbyInfo>();
					for (OrderedAllocationVector<Lobby>::iterator it = m_Server->GetLobbies()->begin(); it != m_Server->GetLobbies()->end(); it++) 
					{
						lobbiesinfo->push_back(DGMTLobbyInfo(it->second->GetLobbyID(),it->second->GetLobbyName(),it->second->GetPlayerCount(),it->second->GetMaxPlayers(),it->second->GetHasPassword(),it->second->GetCreator()->GetDisplayName()));
					}

					Send(m_Protocol->SEND_DGMT_LOBBYLIST(lobbiesinfo));
					delete lobbiesinfo;
					break;
				}
			case DGMTProtocol::DGMT_JOINLOBBY:
				{
					DGMTJoinLobby * info = m_Protocol->RECEIVE_DGMT_JOINLOBBY(packet->GetData());
					if(info == NULL) {PrintMalformedPacket(); break;}
					CONSOLE_Print("[MindTris Server] Player ["+GetDisplayName()+"] trying to join lobby "+UTIL_ToString(info->GetLobbyID())+"...");
					DGMTProtocol::JoinedLobby * answer = new DGMTProtocol::JoinedLobby();
					try{
						Lobby * lobby = m_Server->GetLobbies()->at(info->GetLobbyID());
						if(JoinLobby( lobby,info->GetPassword(),answer))
						{
							SetPublicKey(info->GetPublicKey());
							SetP2PPortNumber(info->GetPortNumber());
							vector<DGMTClientLobbyInfo> * clientinfolist = new vector<DGMTClientLobbyInfo>();
							for (OrderedAllocationVector<User>::iterator it = lobby->GetPeers()->begin(); it != lobby->GetPeers()->end(); it++) 
							{
								User * peer = it->second;
								if(peer !=this)
								{
									clientinfolist->push_back(DGMTClientLobbyInfo(peer->GetPeerID(),peer->GetDisplayName(),peer->GetIPInt32(),peer->GetP2PPortNumber(),peer->GetPublicKey()));
									peer->SendPeerStatusUpdate(this,DGMTProtocol::STATUSUPDATE_HASJOINEDTHELOBBY);
								}
							}
							Send(m_Protocol->SEND_DGMT_JOINEDLOBBY(info->GetLobbyID(),*answer,GetPeerID(),lobby->GetSessionID(),clientinfolist));
							delete clientinfolist;
						}else{
							Send(m_Protocol->SEND_DGMT_JOINEDLOBBY(info->GetLobbyID(),*answer));
						}
					}catch (out_of_range&)
					{
						*answer = DGMTProtocol::JOINEDLOBBY_UNKNOWNERROR;
						Send(m_Protocol->SEND_DGMT_JOINEDLOBBY(info->GetLobbyID(),*answer));
					}
					delete answer;
					break;
				}
			case DGMTProtocol::DGMT_LEAVELOBBY: 
				{
					m_Protocol->RECEIVE_DGMT_LEAVELOBBY(packet->GetData());
					if(LeaveLobby())
					{

					}
					break;
				}
			default:
				CONSOLE_Print( "[MindTris Server] received unhandled packet from [" + m_Socket->GetIPString( ) + "]" ); break;
		}

		delete packet;
	}
	if(m_Error)
	{
		CONSOLE_Print( "[MindTris Server] user [" + m_Socket->GetIPString( ) + "] disconnected: " + m_ErrorString );
		m_DeleteMe = true;
	}
	m_DeleteMe = m_DeleteMe || m_Socket->HasError( ) || !m_Socket->GetConnected( );
	return m_DeleteMe;
}


User :: User(MindTrisServer * nServer, DGMTProtocol *nProtocol, CTCPSocket *nSocket)
{
	m_lobby = NULL;
	m_Protocol = nProtocol;
	m_DeleteMe = false;
	m_Server = nServer;
	m_Error = false;
	m_Socket = nSocket;
}

User :: ~User()
{
	if( m_Socket )
		delete m_Socket;
	if( GetLobby() != NULL )
	{
		if(GetLobby()->GetCreator() == this) delete GetLobby();
	}
	while( !m_Packets.empty( ) )
	{
		delete m_Packets.front( );
		m_Packets.pop( );
	}
}

void User::Send( BYTEARRAY data )
{
	if( m_Socket )
		m_Socket->PutBytes( data );
}