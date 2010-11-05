#include "user.h"
#include "mt_server.h"

bool User :: Update(fd_set * fd){

	if( !m_Socket )
		return true;

	m_Socket->DoRecv( fd );
	string *RecvBuffer = m_Socket->GetBytes( );
	BYTEARRAY Bytes = UTIL_CreateByteArray( (unsigned char *)RecvBuffer->c_str( ), RecvBuffer->size( ) );
	// a packet is at least 7 bytes so loop as long as the buffer contains 7 bytes
	while( Bytes.size( ) >= m_Protocol->DGMT_MINIMALPACKETLENGTH )
	{
		DGMTPacket * packet = NULL;
		if(m_Protocol->ExtractPacket(Bytes, &packet))
		{
			if(packet!=NULL)
			{
				uint16_t Length = packet->GetLength();
				m_Packets.push( packet );
				*RecvBuffer = RecvBuffer->substr( Length );
				Bytes = BYTEARRAY( Bytes.begin( ) + Length, Bytes.end( ) );
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
		uint32_t ProtocolVersion = 0;
		switch( packet->GetID( ) )
		{
			case m_Protocol-> DGMT_HELLOFROMCLIENT:
				 ProtocolVersion = m_Protocol-> RECEIVE_DGMT_HELLOFROMCLIENT( packet->GetData( ) );
				if(ProtocolVersion == m_Protocol->DGMT_PROTOCOLVERSION)
				{
					m_Socket->PutBytes(m_Protocol->SEND_DGMT_HELLOFROMSERVER(m_Protocol->SHELLO_CONNECTED, m_Server->GetMOTD() ));
					CONSOLE_Print( "[MindTris Server] connection accepted from [" + m_Socket->GetIPString( ) + "]" );
				}else{
					m_Socket->PutBytes(m_Protocol->SEND_DGMT_HELLOFROMSERVER(m_Protocol->SHELLO_REFUSEDWRONGVERSION, ""));
					CONSOLE_Print( "[MindTris Server] connection refused from [" + m_Socket->GetIPString( ) + "], wrong protocol version" );
					m_DeleteMe = true;
				}
				break;
			default:
				CONSOLE_Print( "[MindTris Server] received unhandled packet from [" + m_Socket->GetIPString( ) + "]" );
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