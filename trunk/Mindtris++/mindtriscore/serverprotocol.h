#ifndef DGMTPROTOCOL_H
#define DGMTPROTOCOL_H


#define DGMT_HEADER_CONSTANT1 'D'
#define DGMT_HEADER_CONSTANT2 'G'
#define DGMT_HEADER_CONSTANT3 'M'
#define DGMT_HEADER_CONSTANT4 'T'

#define DGMT_PROTOCOL_VERSION 0x00000001

class DGMTConnectionReply;
class DGMTPacket;

class DGMTProtocol
{

public:
	const int static DGMT_MINIMALPACKETLENGTH = 7;
	const uint32_t static DGMT_PROTOCOLVERSION = DGMT_PROTOCOL_VERSION;

	enum Protocol {
		DGMT_HELLOFROMCLIENT		= 0x00,	
		DGMT_CREATEUSER				= 0x01,
		DGMT_LOGIN					= 0x02,
		DGMT_CREATELOBBY			= 0x03,
		DGMT_GETLOBBYLIST			= 0x04,
		DGMT_JOINLOBBY				= 0x05,

		DGMT_HELLOFROMSERVER		= 0x80
	};

	enum ServerHello {

		SHELLO_CONNECTED			= 0x00,
		SHELLO_REFUSEDWRONGVERSION	= 0x01,
		SHELLO_REFUSEDUNKNOWNERROR	= 0x02
	};


	//SEND FUNCTIONS

	BYTEARRAY SEND_DGMT_HELLOFROMCLIENT();
	BYTEARRAY SEND_DGMT_HELLOFROMSERVER(ServerHello answer,string Message);

	//RECEIVE FUNCTIONS
	uint32_t RECEIVE_DGMT_HELLOFROMCLIENT( BYTEARRAY data);
	DGMTConnectionReply * RECEIVE_DGMT_HELLOFROMSERVER( BYTEARRAY data);

	//Other functions
	bool ExtractPacket( BYTEARRAY &Bytes, DGMTPacket ** packet);

	DGMTProtocol(bool bigEndian);
private:
	bool m_isBigEndian;

	bool AssignLength( BYTEARRAY &content );
	bool ValidateLength( BYTEARRAY &content );
	bool AppendHeader( BYTEARRAY &content );

};

class DGMTPacket
{
private:
	int m_ID;
	BYTEARRAY m_Data;
	uint16_t m_Length;
public:
	DGMTPacket( uint16_t nLength, int nID, BYTEARRAY nData );

	int GetID( )					{ return m_ID; }
	uint16_t GetLength( )			{ return m_Length; }
	BYTEARRAY GetData( )			{ return m_Data; }
};

class DGMTConnectionReply
{
	bool m_Connected;
	string m_Message;
public:
	string GetMessage(){return m_Message;}
	bool Connected(){return m_Connected;}
	DGMTConnectionReply(bool nConnected, string nMessage);
};


#endif