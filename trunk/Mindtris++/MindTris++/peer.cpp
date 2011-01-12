
#include <SFML/System.hpp>
#include "mindtriscore/includes.h"
#include "mindtriscore/util.h"
#include "mindtriscore/bytearray.h"
#include "mindtriscore/bytebuffer.h"
#include "mindtriscore/socket.h"
#include "mindtriscore/commprotocol.h"
#include "mindtriscore/packet.h"
#include "mindtriscore/messagestreamer.h"
#include "mindtriscore/serverprotocol.h"
#include "mindtriscore/p2pprotocol.h"
#include "mindtriscore/tetromino.h"

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "state.h"
#include "gfxmanager.h"
#include "sfxmanager.h"

#include "signals.h"
#include "block.h"
#include "mover.h"
#include "moverprovider.h"
#include "board.h"
#include "ingame.h"
#include "globals.h"
#include "tetris.h"

#include "mindtris.h"
#include "peer.h"

Peer :: ~Peer()
{
}

void Peer::NewGame(vector<uint8_t> pieces){
	/*m_tetris.reset(new Tetris( 800, 600, 32, false ));
	for(vector<uint8_t>::iterator iter = pieces.begin(); iter!= pieces.end(); iter++){
		m_tetris->AddNextPiece(*iter);
	}
	m_tetris->run();
	*/
}

// Peer has initiated connection with client
Peer::Peer(MindTrisClient & client, DGMTP2PProtocol & protocol, unique_ptr<CTCPSocket> & nSocket):
m_MessageStreamer(protocol.GetProtocolIdentifier(),protocol.IsBigEndian()),
m_client(client), m_Protocol(protocol)
{
	m_DeleteMe = false;

	m_Error = false;
	m_Socket.swap(nSocket);
	m_ClientSocket = nullptr;
	m_status = STATUS_LISTENING;
}

// Client initiates connection with peer
Peer::Peer(MindTrisClient & client, DGMTP2PProtocol & protocol, uint8_t peerid,const string & displayname, uint32_t ipaddress, uint16_t port, const DSAPublicKey & publickey):
m_MessageStreamer(protocol.GetProtocolIdentifier(),protocol.IsBigEndian()),m_client(client), m_Protocol(protocol)
{
	m_DeleteMe = false;

	m_Error = false;
	m_peerid = peerid;
	m_displayname = displayname;

	m_ClientSocket = new CTCPClient( &CONSOLE_Print, false);
	m_ClientSocket -> Connect( "", ipaddress, port );
	m_Socket.reset(m_ClientSocket);

	m_challengeforpeer = GenerateRandomUINT64(client.GetRandomPool());
	m_challengefrompeer = 0;

	if(!InitializeVerifier(publickey))
	{
		CONSOLE_Print( "[MindTris++] Invalid DSA key from peer: ["+m_displayname+"]" );
	}

	Send(m_Protocol.SEND_CONNECTIONREQUEST(client.GetLobbyID(),client.GetPeerID(),peerid,m_challengeforpeer));

	m_status = STATUS_CONNECTIONREQUEST;
}



void Peer :: PrintMalformedMessage(){
	if(GetConnectionStatus() == STATUS_CONNECTED) CONSOLE_Print( "[MindTris++] Received malformed P2P message from peer ID: "+UTIL_ToString(GetPeerID())+".");
	else CONSOLE_Print( "[MindTris++] Received malformed P2P message from [" + m_Socket->GetIPString( ) + "]" );
}


bool Peer::Update(fd_set *fd)
{

	if( !m_Socket  )
		return true;

	if( !m_Socket->GetConnected()) return false;


	m_Socket->DoRecv( fd );
	if(!m_MessageStreamer.Read(m_Socket->GetRecvBuffer( ),m_IncompletePacket,m_Messages)){
			m_Error = true;
			m_ErrorString = "received invalid packet from peer";
	}

	queue<Message> UnhandledMessages;

	try{
		while( !m_Messages.empty( ) )
		{

			size_t offset = 0;
			Message message = m_Messages.front( );
			m_Messages.pop( );

			byte_t type = m_Protocol.GetMessageType(message,offset);
			switch(GetConnectionStatus())
			{
				case STATUS_LISTENING:
				{
					switch(type){
						case DGMTP2PProtocol::TYPE_CONNECTIONREQUEST:
							{
								DGMTP2PProtocol::ConnectionRequest request = m_Protocol.RECEIVE_CONNECTIONREQUEST(message,offset);
								unique_ptr<PeerInfo> & peerinfo = m_client.GetPeerInfoVector().at(request.GetInitiatingPeerID());
								if(request.GetListeningPeerID() == m_client.GetPeerID() && request.GetLobbyId() == m_client.GetLobbyID()){
									if(peerinfo){
										SetConnectionStatus(STATUS_CONNECTIONACCEPTED);
										SetChallengeFromPeer(request.GetInitiatingChallenge());
										SetChallengeForPeer(GenerateRandomUINT64(m_client.GetRandomPool()));
										SetPeerID(peerinfo->GetPeerID());
										SetDisplayName(peerinfo->GetDisplayName());
										SetPublicKey(peerinfo->GetPublicKey());
										if(!InitializeVerifier(peerinfo->GetPublicKey()))
										{
											CONSOLE_Print( "[MindTris++] Invalid DSA key from peer: ["+peerinfo->GetDisplayName()+"]" );
										}

										Send(m_Protocol.SEND_CONNECTIONACCEPTED(m_client.GetLobbyID(),GetPeerID(),m_client.GetPeerID(),GetChallengeFromPeer(),GetChallengeForPeer(),m_client.GetSigner()));
										CONSOLE_Print("[MindTris++] received a CONNECTION_REQUEST from ["+peerinfo->GetDisplayName()+"]");
									}else{

										UnhandledMessages.push(move(message));
									}
								}else{
									CONSOLE_Print("[MindTris++] received an invalid CONNECTION_REQUEST from ["+GetSocket()->GetIPString()+"]");
									m_DeleteMe = true;
								}
								break;

							}
					}
					break;
				}
				case STATUS_CONNECTIONREQUEST:
				{
					switch(type){
						case DGMTP2PProtocol::TYPE_CONNECTIONACCEPTED:
							{
								DGMTP2PProtocol::ConnectionAcceptAck accept = m_Protocol.RECEIVE_CONNECTIONACCEPTED(message,offset,GetVerifier());
								if( !accept.GetVerified() || accept.GetInitiatingPeerID()!=m_client.GetPeerID() || accept.GetListeningPeerID()!=GetPeerID() || accept.GetLobbyId() != m_client.GetLobbyID() || accept.GetInitiatingChallenge() != GetChallengeForPeer())
								{
									CONSOLE_Print("[MindTris++] Received invalid CONNECTION_ACCEPTED message from ["+GetDisplayName()+"], disconnecting...");
									m_DeleteMe = true;
								}else{
									SetChallengeFromPeer(accept.GetListeningChallenge());
									SetConnectionStatus(STATUS_CONNECTED);
									Send(m_Protocol.SEND_CONNECTIONACKNOWLEDGED(m_client.GetLobbyID(),m_client.GetPeerID(),GetPeerID(),GetChallengeForPeer(),GetChallengeFromPeer(),m_client.GetSigner()));
									CONSOLE_Print("[MindTris++] Received a valid CONNECTION_ACCEPTED message from ["+GetDisplayName()+"], sending CONNECTION_ACKNOWLEDGED");
									CONSOLE_Print("[MindTris++] Successfully connected to ["+GetDisplayName()+"]");
								}
								break;
							}
					}
				}
				case STATUS_CONNECTIONACCEPTED:
				{
					switch(type){
						case DGMTP2PProtocol::TYPE_CONNECTIONACKNOWLEDGED:
							{
								DGMTP2PProtocol::ConnectionAcceptAck accept = m_Protocol.RECEIVE_CONNECTIONACKNOWLEDGED(message,offset,GetVerifier());
								if( !accept.GetVerified() || accept.GetInitiatingPeerID()!=GetPeerID() || accept.GetListeningPeerID()!=m_client.GetPeerID() || accept.GetLobbyId() != m_client.GetLobbyID() || accept.GetInitiatingChallenge() != GetChallengeFromPeer() || accept.GetListeningChallenge() != GetChallengeForPeer())
								{
									CONSOLE_Print("[MindTris++] Received invalid CONNECTION_ACKNOWLEDGED message from ["+GetDisplayName()+"], disconnecting...");
									m_DeleteMe = true;
								}else{
									CONSOLE_Print("[MindTris++] Successfully connected to ["+GetDisplayName()+"]");
									SetConnectionStatus(STATUS_CONNECTED);
								}
								break;
							}
					}
				}
				case STATUS_CONNECTED:
				{
					switch(type){
						case DGMTP2PProtocol::TYPE_CHATSEND:
							{
								DGMTP2PProtocol::ChatSend info = m_Protocol.RECEIVE_CHATSEND(message,offset,GetVerifier());
								CONSOLE_Print("[P2P Chat] "+GetDisplayName()+": "+info.GetMessage());					
								break;
							}
						case DGMTP2PProtocol::TYPE_ROUND:
							{
								DGMTP2PProtocol::RoundData info = m_Protocol.RECEIVE_ROUND(message,offset,GetVerifier());
			
								break;
							}
					}
					break;
				}
			}
			m_Messages = move(UnhandledMessages);

		}
	}catch (DGMTP2PProtocol::Err p)
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

void Peer::SendRoundData(uint64_t sessionid, uint32_t round, vector<DGMTP2PProtocol::Move> moves, const unique_ptr<CryptoPP::DSA::Signer> & signer){
	Send( m_Protocol.SEND_ROUND(sessionid,round,moves,vector<DGMTP2PProtocol::RoundDataHash>(),signer));
}

void Peer::SendChat(uint64_t sessionid, const string & msg, const unique_ptr<CryptoPP::DSA::Signer> & signer){
	Send( m_Protocol.SEND_CHATSEND(sessionid,msg,signer));
}

void Peer::Send( Message && msg )
{
	m_MessageStreamer.Write(m_Socket->GetSendBuffer(), forward<Message>(msg));
}

bool Peer::InitializeVerifier(const DSAPublicKey & key)
{
		m_publickey = key;
		string p(key.GetP());
		string q(key.GetQ());
		string g(key.GetG());
		string y(key.GetY());

		CryptoPP::Integer i_p,i_q,i_g,i_y;

		i_p.Decode(CryptoPP::StringSource(p,true),p.size(),CryptoPP::Integer::UNSIGNED);
		i_q.Decode(CryptoPP::StringSource(q,true),q.size(),CryptoPP::Integer::UNSIGNED);
		i_g.Decode(CryptoPP::StringSource(g,true),g.size(),CryptoPP::Integer::UNSIGNED);
		i_y.Decode(CryptoPP::StringSource(y,true),y.size(),CryptoPP::Integer::UNSIGNED);

		CryptoPP::DSA::PublicKey pubkey;
		pubkey.Initialize(i_p,i_q,i_g,i_y);

		m_Verifier.reset(new CryptoPP::DSA::Verifier(pubkey));
		if (!pubkey.Validate(m_client.GetRandomPool(), 3)){return false;}
		return true;
}