
#include "includes.h"
#include "util.h"
#include "serverprotocol.h"

///////////////////////
// SEND FUNCTIONS    //
///////////////////////

BYTEARRAY DGMTProtocol::SEND_DGMT_HELLOFROMCLIENT()
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( DGMT_HELLOFROMCLIENT );	// DGMT_HELLOFROMCLIENT
	UTIL_AppendByteArray(packet,DGMTProtocol::DGMT_PROTOCOLVERSION,m_isBigEndian);
	AssignLength( packet );

	return packet;
}

BYTEARRAY DGMTProtocol::SEND_DGMT_HELLOFROMSERVER(ServerHello answer,string Message)
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( DGMT_HELLOFROMSERVER );	// HELLO FROM SERVER
	packet.push_back( answer );	// server reply
	UTIL_AppendByteArray(packet, Message,m_isBigEndian);
	AssignLength( packet );

	return packet;
}


///////////////////////
// RECEIVE FUNCTIONS //
///////////////////////


uint32_t DGMTProtocol::RECEIVE_DGMT_HELLOFROMCLIENT(BYTEARRAY data)
{
	if( ValidateLength(data) && data.size() >=11)
	{
		return UTIL_ByteArrayToUInt32( data, m_isBigEndian, 7 );
	}
	return (uint32_t) 0;
}


DGMTConnectionReply * DGMTProtocol::RECEIVE_DGMT_HELLOFROMSERVER(BYTEARRAY data)
{
	if( ValidateLength(data) && data.size() >=8)
	{

		BYTEARRAY Message;
		string MessageS;

		switch(data[7])
		{
			case SHELLO_CONNECTED:
			/*	 Message = BYTEARRAY(, ); */
				 return new DGMTConnectionReply(true, string( data.begin( ) + 8,  data.end( )));
				 break;
			case SHELLO_REFUSEDWRONGVERSION:
				 UTIL_AppendByteArray(UTIL_CreateByteArray("Server refused connection. Wrong client version:"), BYTEARRAY( data.begin( ) + 8, data.end( ) ));
				 return new DGMTConnectionReply(false, string(Message.begin(), Message.end()));
				 break;
			case SHELLO_REFUSEDUNKNOWNERROR:
				 return new DGMTConnectionReply(false, string(data.begin( ) + 8,data.end( )));
				 break;
		}
	}
	return NULL;
}



/////////////////////
// OTHER FUNCTIONS //
/////////////////////

bool DGMTProtocol :: ExtractPacket( BYTEARRAY &Bytes, DGMTPacket ** packet)
{
	if (Bytes[0] == DGMT_HEADER_CONSTANT1 && Bytes[1] == DGMT_HEADER_CONSTANT2 && Bytes[2] == DGMT_HEADER_CONSTANT3 && Bytes[3] == DGMT_HEADER_CONSTANT4){
		uint16_t Length  = UTIL_ByteArrayToUInt16( Bytes, m_isBigEndian, 4 );
		if(Length >= DGMT_MINIMALPACKETLENGTH)
		{
			if(Length <= Bytes.size()){
				*packet = new DGMTPacket(Length, Bytes[6], BYTEARRAY( Bytes.begin( ), Bytes.begin( ) + Length ));
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


bool DGMTProtocol :: AppendHeader( BYTEARRAY &content)
{
	content.push_back(DGMT_HEADER_CONSTANT1);
	content.push_back(DGMT_HEADER_CONSTANT2);
	content.push_back(DGMT_HEADER_CONSTANT3);
	content.push_back(DGMT_HEADER_CONSTANT4);
	return true;
}

bool DGMTProtocol :: AssignLength( BYTEARRAY &content )
{
	// insert the actual length of the content array into bytes 3 and 4 (indices 2 and 3)

	BYTEARRAY LengthBytes;

	if( content.size( ) >= 6 && content.size( ) <= 65535 )
	{
		LengthBytes = UTIL_CreateByteArray( (uint16_t)content.size( ), m_isBigEndian);
		content[4] = LengthBytes[0];
		content[5] = LengthBytes[1];
		return true;
	}

	return false;
}

bool DGMTProtocol :: ValidateLength( BYTEARRAY &content )
{
	// verify that bytes 3 and 4 (indices 2 and 3) of the content array describe the length

	uint16_t Length;
	BYTEARRAY LengthBytes;

	if( content.size( ) >= 6 && content.size( ) <= 65535 )
	{
		LengthBytes.push_back( content[4] );
		LengthBytes.push_back( content[5] );
		Length = UTIL_ByteArrayToUInt16( LengthBytes, m_isBigEndian );

		if( Length == content.size( ) )
			return true;
	}

	return false;
}

DGMTProtocol :: DGMTProtocol(bool bigEndian)
{
	m_isBigEndian = true;
}


DGMTConnectionReply::DGMTConnectionReply(bool nConnected , string nMessage)
{
	m_Connected = nConnected;
	m_Message = nMessage;
}

DGMTPacket:: DGMTPacket( uint16_t nLength, int nID, BYTEARRAY nData )
{
	m_Length = nLength;
	m_ID = nID;
	m_Data = nData;
}