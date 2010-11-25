#include "mindtriscore/includes.h"
#include "mindtriscore/util.h"
#include "mindtriscore/socket.h"
#include "mindtriscore/serverprotocol.h"
#include "mindtriscore/p2pprotocol.h"
#include "mindtris.h"
#include "peer.h"

Peer :: ~Peer()
{
	if( m_Socket )
		delete m_Socket;
	while( !m_Packets.empty( ) )
	{
		delete m_Packets.front( );
		m_Packets.pop( );
	}
}


// Peer has initiated connection with client
Peer::Peer(MindTrisClient * client, DGMTP2PProtocol * protocol, CTCPSocket * nSocket){
	m_Protocol = protocol;
	m_DeleteMe = false;
	m_client = client;
	m_Error = false;
	m_Socket = nSocket;

	m_peerinitiatedhandshake = true;
	m_clientinitiatedhandshake = false;

	m_trusted = false;
}

// Client initiates connection with peer
Peer::Peer(MindTrisClient * client, DGMTP2PProtocol * protocol, uint8_t peerid, string displayname, uint32_t ipaddress, uint16_t port, RSAPublicKey * publickey){
	m_Protocol = protocol;
	m_DeleteMe = false;
	m_client = client;
	m_Error = false;
	m_peerid = peerid;
	m_displayname = displayname;
	m_publickey = publickey;
	m_ClientSocket  = new CTCPClient( &CONSOLE_Print, false);
	m_ClientSocket -> Connect( "", ipaddress, port );

	m_Socket = m_ClientSocket;
	m_Socket->PutBytes(protocol->SEND_HELLOFROMPEER(client->GetPeerID(),client->GetLobbyID()));

	m_peerinitiatedhandshake = false;
	m_clientinitiatedhandshake = true;
	m_trusted = false;
}


void Peer :: PrintMalformedPacket(){
	if(m_trusted) CONSOLE_Print( "[MindTris++] Received malformed P2P packet from peer ID: "+UTIL_ToString(m_peerid)+".");
	else CONSOLE_Print( "[MindTris++] Received malformed P2P packet from [" + m_Socket->GetIPString( ) + "]" );
}


bool Peer::Update(fd_set *fd)
{

	if( !m_Socket  )
		return true;
	if( !m_Socket->GetConnected()) return false;
	m_Socket->DoRecv( fd );
	string * buffer = m_Socket->GetBytes( );
	BYTEARRAY Bytes(buffer->begin(),buffer->end());

	while( Bytes.size() >= DGMTP2PProtocol::HEADERLENGTH )
	{

		DGMTP2PProtocol::Packet * packet = NULL;
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
			m_ErrorString = "received invalid packet from peer";
			break;
		}
	}

	queue<DGMTP2PProtocol::Packet *> UnhandledPackets = queue<DGMTP2PProtocol::Packet *>();

	while( !m_Packets.empty( ) )
	{
		DGMTP2PProtocol::Packet *packet = m_Packets.front( );

		m_Packets.pop( );

		switch( packet->GetID( ) )
		{
			case DGMTP2PProtocol::DGMTP2P_HELLOFROMPEER:
				 {
					DGMTP2PProtocol::HelloFromPeer * reply = m_Protocol-> RECEIVE_HELLOFROMPEER(packet->GetData());
					if(reply==NULL) {PrintMalformedPacket(); break;}

					if(!m_trusted)
					{
						if(m_clientinitiatedhandshake)
						{
							if( reply->GetPeerID()== GetPeerID() && reply->GetLobbyId() == m_client->GetLobbyID()){
								CONSOLE_Print("[MindTris++] Established handshake with peer ["+GetDisplayName()+"]");
								m_trusted = true;
							}else{
								m_DeleteMe = true;
							}
						}else if (m_peerinitiatedhandshake)	{
							PeerInfo * peerinfo = m_client->GetPeerInfoVector()->at(reply->GetPeerID());
							if(peerinfo!=NULL){
								if(peerinfo->GetPeerID() == reply->GetPeerID() && reply->GetLobbyId() == m_client->GetLobbyID())
								{
									m_trusted = true;
									SetPeerID(peerinfo->GetPeerID());
									SetDisplayName(peerinfo->GetDisplayName());
									SetPublicKey(peerinfo->GetPublicKey());
									m_Socket->PutBytes(m_Protocol-> SEND_HELLOFROMPEER(m_client->GetPeerID(),m_client->GetLobbyID()));
									CONSOLE_Print("[MindTris++] Established handshake with peer ["+peerinfo->GetDisplayName()+"]");
								}else{
									m_DeleteMe = true;
								}
							}else{
								UnhandledPackets.push(packet);
								packet = NULL;
							}
						}
					} 
					break;
				 }
			case DGMTP2PProtocol::DGMTP2P_CHATSEND:
				{
					if(m_trusted){
						DGMTP2PProtocol::ChatSend * info = m_Protocol-> RECEIVE_CHATSEND(packet->GetData());
						if(info == NULL) {PrintMalformedPacket(); break;}
						CONSOLE_Print("[P2P Chat] "+GetDisplayName()+": "+info->GetMessageW());					
						delete info;
						break;
					}
				}
		}
		if(packet) delete packet;
	}

	while (!UnhandledPackets.empty() )
	{
		m_Packets.push(UnhandledPackets.front());
		UnhandledPackets.pop();
	}
	
	if(m_Error)
	{
		CONSOLE_Print( "[MindTris Server] user [" + m_Socket->GetIPString( ) + "] disconnected: " + m_ErrorString );
		m_DeleteMe = true;
	}
	m_DeleteMe = m_DeleteMe || m_Socket->HasError( ) || !m_Socket->GetConnected( );
	return m_DeleteMe;
}

