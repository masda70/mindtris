
#include "includes.h"
#include "util.h"
#include "p2pprotocol.h"

///////////////////////
// SEND FUNCTIONS    //
///////////////////////


BYTEARRAY DGMTP2PProtocol::SEND_KEEPALIVE()
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	AssignLength( packet );
	return packet;
}

BYTEARRAY DGMTP2PProtocol::SEND_CHATSEND(string signature, string message)
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMTP2P_CHATSEND);			// DGMTP2P_CHATSEND
	
	UTIL_AppendByteArray(packet, (uint16_t) signature.length(), m_isBigEndian);
	UTIL_AppendByteArray(packet, signature, false);

	UTIL_AppendByteArray(packet, (uint16_t) message.length(), m_isBigEndian);
	UTIL_AppendByteArray(packet, message, false);

	AssignLength( packet );
	return packet;
}

BYTEARRAY DGMTP2PProtocol::SEND_HELLOFROMPEER(uint8_t clientid, uint32_t lobbyid)
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMTP2P_HELLOFROMPEER );			// DGMTP2P_HELLOFROMPEER
	
	UTIL_AppendByteArray(packet, (uint8_t) clientid, m_isBigEndian);
	UTIL_AppendByteArray(packet, (uint32_t) lobbyid, m_isBigEndian);

	AssignLength( packet );
	return packet;
}

///////////////////////
// RECEIVE FUNCTIONS //
///////////////////////
	
DGMTP2PProtocol::HelloFromPeer * DGMTP2PProtocol::RECEIVE_HELLOFROMPEER(BYTEARRAY data)
{
	int offset = 0;
	int length;
	int datasize = data.size( );

		length = 1;
		if(!( datasize >= length + offset)) return NULL;
		uint8_t peerid = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
		offset+= length;

		length = 4;
		if(!( datasize >= length + offset)) return NULL;
		uint32_t lobbyid= UTIL_ByteArrayToUInt32( data, m_isBigEndian, offset );
		offset+= length;
		
		return new HelloFromPeer(peerid,lobbyid);
}
DGMTP2PProtocol::ChatSend * DGMTP2PProtocol::RECEIVE_CHATSEND(BYTEARRAY data)
{

	int offset = 0;
	int length;
	int datasize = data.size( );

		length = 2;
		if(!( datasize >= length + offset)) return NULL;
		uint16_t signature_length= UTIL_ByteArrayToUInt16( data, m_isBigEndian, offset );
		offset+= length;

		length =  signature_length;
		if(!(datasize >= length + offset)) return NULL;
		string signature = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset+= length;

		length = 2;
		if(!( datasize >= length + offset)) return NULL;
		uint16_t message_length= UTIL_ByteArrayToUInt16( data, m_isBigEndian, offset );
		offset+= length;

		length =  message_length;
		if(!(datasize >= length + offset)) return NULL;
		string message = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset+= length;

		return new ChatSend(signature,message);
}


/////////////////////
// OTHER FUNCTIONS //
/////////////////////

bool DGMTP2PProtocol :: ExtractPacket( BYTEARRAY Bytes, Packet ** packet)
{
	if (Bytes[0] == DGMTP2P_HEADER_CONSTANT1 && Bytes[1] == DGMTP2P_HEADER_CONSTANT2 && Bytes[2] == DGMTP2P_HEADER_CONSTANT3 && Bytes[3] == DGMTP2P_HEADER_CONSTANT4 && Bytes[4] == DGMTP2P_HEADER_CONSTANT5 && Bytes[5] == DGMTP2P_HEADER_CONSTANT6 && Bytes[6] == DGMTP2P_HEADER_CONSTANT7){
		uint16_t Length  = UTIL_ByteArrayToUInt16( Bytes, m_isBigEndian, 7 );
		if(Length >= HEADERLENGTH)
		{
			if(Length <= Bytes.size()){
				if(Length == HEADERLENGTH)
				{
					*packet = new Packet(Length, DGMTP2P_KEEPALIVE, BYTEARRAY( Bytes.begin( ) + HEADERLENGTH+1, Bytes.begin( ) + Length ));
				}else{
					*packet = new Packet(Length, Bytes[HEADERLENGTH], BYTEARRAY( Bytes.begin( ) + HEADERLENGTH+1, Bytes.begin( ) + Length ));
				}
				return true;
			}
			else
			{
				*packet = NULL;
				return true;
			}
		}
		else
		{
			return false;
		}
	}else{
		return false;
	}
}


bool DGMTP2PProtocol :: AppendHeader( BYTEARRAY &content)
{
	content.push_back(DGMTP2P_HEADER_CONSTANT1);
	content.push_back(DGMTP2P_HEADER_CONSTANT2);
	content.push_back(DGMTP2P_HEADER_CONSTANT3);
	content.push_back(DGMTP2P_HEADER_CONSTANT4);
	content.push_back(DGMTP2P_HEADER_CONSTANT5);
	content.push_back(DGMTP2P_HEADER_CONSTANT6);
	content.push_back(DGMTP2P_HEADER_CONSTANT7);
	content.push_back( 0 );						// packet length will be assigned later
	content.push_back( 0 );						// packet length will be assigned later
	return true;
}

bool DGMTP2PProtocol :: AssignLength( BYTEARRAY &content )
{
	// insert the actual length of the content array into bytes 3 and 4 (indices 2 and 3)

	BYTEARRAY LengthBytes;

	if( content.size( ) >= HEADERLENGTH && content.size( ) <= 65535 )
	{
		LengthBytes = UTIL_CreateByteArray( (uint16_t)content.size( ), m_isBigEndian);
		content[7] = LengthBytes[0];
		content[8] = LengthBytes[1];
		return true;
	}

	return false;
}


DGMTP2PProtocol :: DGMTP2PProtocol(bool bigEndian)
{
	m_isBigEndian = true;
}

