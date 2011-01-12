
#include "mindtriscore/includes.h"
#include "mindtriscore/util.h"
#include "mindtriscore/bytearray.h"
#include "mindtriscore/bytebuffer.h"
#include "mindtriscore/socket.h"
#include "mindtriscore/oalloc.h"
#include "mindtriscore/commprotocol.h"
#include "mindtriscore/packet.h"
#include "mindtriscore/messagestreamer.h"
#include "mindtriscore/serverprotocol.h"
#include "mindtriscore/p2pprotocol.h"
#include "database.h"
#include "mt_server.h"
#include "lobby.h"
#include "user.h"


uint32_t User :: GetIPInt32() const{ return m_Socket->GetIPInt32();}

void User :: SendPeerStatusUpdate(User & u, DGMTProtocol::UpdateClientStatus::status status)
{
	switch(status)
	{
		case DGMTProtocol::UpdateClientStatus::STATUSUPDATE_HASJOINEDTHELOBBY:
			{
				//u->GetDisplayName()
				string displayname = u.GetDisplayName();
				Send(m_Protocol.SEND_UPDATECLIENTSTATUS(status,this->GetLobby()->GetLobbyID(),u.GetPeerID(),u.GetDisplayName(),u.GetIPInt32(),u.GetP2PPortNumber(),u.GetPublicKey()));
				break;
			}
		case DGMTProtocol::UpdateClientStatus::STATUSUPDATE_HASLEFTTHELOBBY:
			{
				Send(m_Protocol.SEND_UPDATECLIENTSTATUS(status,this->GetLobby()->GetLobbyID(),u.GetPeerID()));
				break;
			}
		case DGMTProtocol::UpdateClientStatus::STATUSUPDATE_HASBEENKICKED:
			{
				this->Send(m_Protocol.SEND_UPDATECLIENTSTATUS(status,this->GetLobby()->GetLobbyID(),u.GetPeerID()));
				break;
			}
	}
}

void User :: PrintMalformedMessage(){
	CONSOLE_Print( "[MindTris Server] Received malformed packet from [" + m_Socket->GetIPString( ) + "]" );
}

bool User :: Update(fd_set * fd){

	if( !m_Socket )
		return true;
	m_Socket->DoRecv( fd );
	if(!m_MessageStreamer.Read(m_Socket->GetRecvBuffer( ),m_IncompletePacket,m_Messages)){
			m_Error = true;
			m_ErrorString = "received invalid packet from user";
	}

	try{
		while( !m_Messages.empty( ) )
		{

			size_t offset = 0;
			Message message = m_Messages.front( );
			m_Messages.pop( );
			byte_t type;
			if(message.size() == 0)
			{
				type = DGMTProtocol::TYPE_KEEPALIVE;
			}else{
				type = m_Protocol.GetMessageType(message,offset);
			}
			switch( type)
			{
			case DGMTProtocol::TYPE_KEEPALIVE:
				{

					// To do: KEEP ALIVE 
					break;
				}
			case  DGMTProtocol::TYPE_HELLOFROMCLIENT:
					 {
						uint32_t ProtocolVersion = 0;
						ProtocolVersion = m_Protocol.RECEIVE_HELLOFROMCLIENT(message,offset);
						if(ProtocolVersion == m_Protocol.PROTOCOLVERSION)
						{
							Send(m_Protocol.SEND_HELLOFROMSERVER(DGMTProtocol::SHELLO_CONNECTED,  m_Server.GetPublicKey(), m_Server.GetMOTD() ));
							CONSOLE_Print( "[MindTris Server] connection accepted from [" + m_Socket->GetIPString( ) + "]" );
						}else{
							Send(m_Protocol.SEND_HELLOFROMSERVER(DGMTProtocol::SHELLO_REFUSEDWRONGVERSION,  m_Server.GetPublicKey(), ""));
							CONSOLE_Print( "[MindTris Server] connection refused from [" + m_Socket->GetIPString( ) + "], wrong protocol version" );
							m_DeleteMe = true;
						}
						break;
					 }
				case DGMTProtocol::TYPE_CREATEUSER:
					{

						DGMTProtocol::CreateUserInfo info = m_Protocol.RECEIVE_CREATEUSER( message,offset, *m_Server.GetDecryptor());
						if(m_Server.GetDatabase().UserExists(info.GetUsername()))
						{
							Send(m_Protocol.SEND_USERCREATION(DGMTProtocol::USERCREATION_USERNAMEALREADYEXISTS));
						}else{
							m_Server.GetDatabase().AddUser(info.GetUsername(),info.GetDisplayName(),info.GetEmail(),info.GetPassword());
							Send(m_Protocol.SEND_USERCREATION(m_Protocol.USERCREATION_SUCCESS));
							CONSOLE_Print("[MindTris Server] Created User ["+info.GetUsername()+"].");
						}
						break;
					}
				case DGMTProtocol::TYPE_LOGIN:
					{
						DGMTProtocol::LoginInfo info = m_Protocol.RECEIVE_LOGIN(message,offset, *m_Server.GetDecryptor());
						unique_ptr<UserData> userdata;
						if(m_Server.GetDatabase().UserMatchesPassword(info.GetUsername(),info.GetPassword(), userdata))
						{ 
							CONSOLE_Print("[MindTris Server] User ["+info.GetUsername()+"] has logged in.");
							SetDisplayName(userdata->GetDisplayName());
							Send(m_Protocol.SEND_LOGINREPLY(DGMTProtocol::LoginReply::LOGINREPLY_SUCCESS,userdata->GetDisplayName()));
						}else{
							if(userdata != NULL)
							{
								CONSOLE_Print("[MindTris Server] Client ["+ m_Socket->GetIPString( )+"] log in: password/username mismatch.");
								Send(m_Protocol.SEND_LOGINREPLY(DGMTProtocol::LoginReply::LOGINREPLY_BADUSERNAMEPASSWORD,""));

							}else{
								CONSOLE_Print("[MindTris Server] Client ["+ m_Socket->GetIPString( )+"] log in: username ["+info.GetUsername()+"] does not exist.");
								Send(m_Protocol.SEND_LOGINREPLY(DGMTProtocol::LoginReply::LOGINREPLY_USERNAMEDOESNOTEXIST,""));
							}
						}
						break;
					}
				case DGMTProtocol::TYPE_CREATELOBBY:
					{

						DGMTProtocol::CreateLobby info = m_Protocol. RECEIVE_CREATELOBBY(message,offset,*m_Server. GetDecryptor());
						CONSOLE_Print("[MindTris Server] Player ["+GetDisplayName()+"] trying to create lobby "+info.GetLobbyName()+".");
						if(info.GetMaxPlayers()<=0) 
						{
							Send(m_Protocol.SEND_LOBBYCREATION(DGMTProtocol::LobbyCreation::LOBBYCREATION_INVALIDNUMBEROFPLAYERS));
						}else{
							if(GetLobby())
							{
								Send(m_Protocol.SEND_LOBBYCREATION(DGMTProtocol::LobbyCreation::LOBBYCREATION_UNKNOWNERROR));
							}else{
								Lobby * l= m_Server.CreateLobby(*this, info.GetLobbyName(),info.GetMaxPlayers(),info.GetHasPassword(),info.GetPassword());
								DGMTProtocol::JoinedLobby::answer answer;
								if(l->UserJoin(*this, info.GetPassword(), answer))
								{
									SetLobby(l); 
									SetPublicKey(info.GetPublicKey());
									SetP2PPortNumber(info.GetPortNumber());
									Send(m_Protocol.SEND_LOBBYCREATION(DGMTProtocol::LobbyCreation::LOBBYCREATION_SUCCESS,l->GetLobbyID(),GetPeerID(),l->GetSessionID()));
								}else
								{
									Send(m_Protocol.SEND_LOBBYCREATION(DGMTProtocol::LobbyCreation::LOBBYCREATION_UNKNOWNERROR));
								}
							}
						}
						break;
					}
				case DGMTProtocol::TYPE_GETLOBBYLIST:
					{
						m_Protocol. RECEIVE_GETLOBBYLIST(message,offset);
						CONSOLE_Print("[MindTris Server] Player ["+GetDisplayName()+"] trying to retrieve lobby list.");
						vector<DGMTProtocol::LobbyInfo> lobbiesinfo;
						for (LobbyList::iterator it = m_Server.GetLobbies().begin(); it != m_Server.GetLobbies().end(); it++) 
						{
							lobbiesinfo.push_back(DGMTProtocol::LobbyInfo(it->second->GetLobbyID(),it->second->GetLobbyName(),it->second->GetPlayerCount(),it->second->GetMaxPlayers(),it->second->GetHasPassword(),it->second->GetCreator().GetDisplayName()));
						}

						Send(m_Protocol.SEND_LOBBYLIST(lobbiesinfo));
						break;
					}
				case DGMTProtocol::TYPE_JOINLOBBY:
					{
						DGMTProtocol::JoinLobby info = m_Protocol.RECEIVE_JOINLOBBY(message,offset);
						CONSOLE_Print("[MindTris Server] Player ["+GetDisplayName()+"] trying to join lobby "+UTIL_ToString(info.GetLobbyID())+"...");
						if(GetLobby()){
							Send(m_Protocol.SEND_JOINEDLOBBY(info.GetLobbyID(),DGMTProtocol::JoinedLobby::JOINEDLOBBY_UNKNOWNERROR));
						}else{
							DGMTProtocol::JoinedLobby::answer answer;
							try{
								Lobby * lobby = m_Server.GetLobbies().at(info.GetLobbyID());
								if(!lobby) throw out_of_range("");
								if(lobby->UserJoin(*this, info.GetPassword(), answer))
								{
									SetLobby(lobby); 
									SetPublicKey(info.GetPublicKey());
									SetP2PPortNumber(info.GetPortNumber());
									vector<DGMTProtocol::ClientLobbyInfo> clientinfolist;
									for (UserList::iterator it = lobby->GetPeers().begin(); it != lobby->GetPeers().end(); it++) 
									{
										User & peer = *it->second;
										if(&peer != this)
										{
											clientinfolist.push_back(DGMTProtocol::ClientLobbyInfo(peer.GetPeerID(),peer.GetDisplayName(),peer.GetIPInt32(),peer.GetP2PPortNumber(),peer.GetPublicKey()));
											peer.SendPeerStatusUpdate(*this,DGMTProtocol::UpdateClientStatus::STATUSUPDATE_HASJOINEDTHELOBBY);
										}
									}
									Send(m_Protocol.SEND_JOINEDLOBBY(info.GetLobbyID(),answer,lobby->GetLobbyName(),lobby->GetMaxPlayers(),lobby->GetCreator().GetPeerID(),GetPeerID(),lobby->GetSessionID(),clientinfolist));
								}else{
									Send(m_Protocol.SEND_JOINEDLOBBY(info.GetLobbyID(),answer));
								}
							}catch(out_of_range r)
							{
								Send(m_Protocol.SEND_JOINEDLOBBY(info.GetLobbyID(),DGMTProtocol::JoinedLobby::JOINEDLOBBY_UNKNOWNERROR));
							}catch (int e)
							{
								Send(m_Protocol.SEND_JOINEDLOBBY(info.GetLobbyID(),DGMTProtocol::JoinedLobby::JOINEDLOBBY_UNKNOWNERROR));
							}
							break;
						}
					}

				case DGMTProtocol::TYPE_LEAVELOBBY: 
					{
						m_Protocol.RECEIVE_LEAVELOBBY(message,offset);
						CONSOLE_Print("[MindTris Server] Player ["+GetDisplayName()+"] trying to leave lobby "+UTIL_ToString(GetLobby()->GetLobbyID())+"...");
						if(this->GetLobby()){
							if(&GetLobby()->GetCreator() == this){
								m_Server.DestroyLobby(GetLobby());
							}else{
								if(this->GetLobby()->UserLeave(*this)) SetLobby( nullptr);
							}
						}
						break;
					}
				case DGMTProtocol::TYPE_STARTGAME:
					{
						m_Protocol.RECEIVE_STARTGAME(message,offset);
						if(this->GetLobby()){
							if(&GetLobby()->GetCreator() == this){
								CONSOLE_Print("[MindTris Server] Player ["+GetDisplayName()+"] trying to start lobby ID "+UTIL_ToString(GetLobby()->GetLobbyID())+"...");
								Send(m_Protocol.SEND_GAMESTARTING(DGMTProtocol::GAMESTARTING_STARTING));

								GetLobby()->SetGameStarting(true);
								ByteArray msg = m_Protocol.SEND_LOADGAME(GetLobby()->GetNextPieces(m_Server.GetRandomGenerator(),7));
								for (UserList::iterator it = GetLobby()->GetPeers().begin(); it != GetLobby()->GetPeers().end(); it++) 
								{
									User & peer = *it->second;
									ByteArray msg_copy = msg;
									peer.Send(move(msg_copy));
								}
							}
						}
						break;
					}
				case DGMTProtocol::TYPE_LOADEDGAME:
					{
						DGMTProtocol::LoadedGame info = m_Protocol.RECEIVE_LOADEDGAME(message,offset);
						if(this->GetLobby()){
							if(GetLobby()->GetGameStarting()){
								CONSOLE_Print("[MindTris Server] Player ["+GetDisplayName()+"] has loaded game from lobby ID "+UTIL_ToString(GetLobby()->GetLobbyID())+"...");
								SetHasLoadedGame(true);
								bool start = true;
								for (UserList::iterator it = GetLobby()->GetPeers().begin(); it != GetLobby()->GetPeers().end(); it++) 
								{
									User & peer = *it->second;
									
									if(!peer.HasLoadedGame()){
										start = false; break;
									}
								}
								if(start){
									CONSOLE_Print("[MindTris Server] Started lobby ID "+UTIL_ToString(GetLobby()->GetLobbyID())+"...");
									GetLobby()->SetGameStarted(true);
									for (UserList::iterator it = GetLobby()->GetPeers().begin(); it != GetLobby()->GetPeers().end(); it++) 
									{
										User & peer = *it->second;
									
										peer.Send(m_Protocol.SEND_BEGINGAME());
									}
								}
							}

						}
						break;
					}
				case DGMTProtocol::TYPE_GIVENEWPIECES:
					{
						DGMTProtocol::GiveNewPieces info = m_Protocol.RECEIVE_GIVENEWPIECES(message,offset);
						if(this->GetLobby()){
							if(GetLobby()->GetGameStarted()){
								CONSOLE_Print("[MindTris Server] Player ["+GetDisplayName()+"] is requesting "+UTIL_ToString(info.GetNumber())+" more pieces at offset "+ UTIL_ToString(info.GetOffset()));
								if(info.GetOffset() == GetLobby()->GetPieceOffset())
								{
									ByteArray msg = m_Protocol.SEND_NEWPIECES(info.GetOffset(),GetLobby()->GetNextPieces(m_Server.GetRandomGenerator(),info.GetNumber()));
									for (UserList::iterator it = GetLobby()->GetPeers().begin(); it != GetLobby()->GetPeers().end(); it++) 
									{
										User & peer = *it->second;
										ByteArray msg_copy = msg;
										peer.Send(move(msg_copy));
									}
								}
							}
						}
						break;
					}
				default:
					CONSOLE_Print( "[MindTris Server] received unhandled packet from [" + m_Socket->GetIPString( ) + "]" ); break;
			}
		}
	}catch (DGMTProtocol::Err p)
	{
		PrintMalformedMessage(); m_Error = true;
	}catch (MessageParser::malformed_message p)
	{
		PrintMalformedMessage(); m_Error = true;
	}

	if(m_Error)
	{
		CONSOLE_Print( "[MindTris Server] user [" + m_Socket->GetIPString( ) + "] disconnected: " + m_ErrorString );
		m_DeleteMe = true;
	}
	m_DeleteMe = m_DeleteMe || m_Socket->HasError( ) || !m_Socket->GetConnected( );
	return m_DeleteMe;
}


User :: User(MindTrisServer & nServer, DGMTProtocol & nProtocol, unique_ptr<CTCPSocket> nSocket) :
m_Protocol(nProtocol), m_MessageStreamer(nProtocol.GetProtocolIdentifier(),nProtocol.IsBigEndian()), m_Server(nServer)
{
	m_loadedgame = false;
	m_lobby = nullptr;
	m_DeleteMe = false;
	m_Error = false;
	m_Socket.swap(nSocket);

}

User :: ~User()
{
	if(this->GetLobby()){
		if(&GetLobby()->GetCreator() == this){
			m_Server.DestroyLobby(GetLobby());
		}else{
			if(this->GetLobby()->UserLeave(*this)) SetLobby( nullptr);
		}
	}
}

void User::Send( Message && msg )
{
	m_MessageStreamer.Write(m_Socket->GetSendBuffer(), forward<Message>(msg));
}