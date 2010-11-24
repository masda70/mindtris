#ifndef DGMTP2PPROTOCOL_H
#define DGMTP2PPROTOCOL_H

#include "includes.h"

#define DGMTP2P_HEADER_CONSTANT1 'D'
#define DGMTP2P_HEADER_CONSTANT2 'G'
#define DGMTP2P_HEADER_CONSTANT3 'M'
#define DGMTP2P_HEADER_CONSTANT4 'T'
#define DGMTP2P_HEADER_CONSTANT5 'P'
#define DGMTP2P_HEADER_CONSTANT6 '2'
#define DGMTP2P_HEADER_CONSTANT7 'P'


class DGMTP2PPacket;

class DGMTP2PProtocol
{

public:
	const int static HEADERLENGTH = 9;

	enum Protocol {
		DGMTP2P_KEEPALIVE				= 0xFF,
		DGMTP2P_HELLOFROMPEER		= 0x00,	
		DGMTP2P_CHATSEND			= 0x01,
	};

	//SEND FUNCTIONS
	BYTEARRAY SEND_KEEPALIVE();

		// HELLOFROMPEER
		BYTEARRAY SEND_HELLOFROMPEER(uint8_t clientid, uint32_t lobbyid);
		BYTEARRAY SEND_CHATSEND(string signature, string message);

	//RECEIVE FUNCTIONS

	class HelloFromPeer
	{
	private:
		uint8_t m_peerid;
		uint32_t m_lobbyid;
	public:
		uint8_t GetPeerID(){return m_peerid;}
		uint32_t GetLobbyId(){return m_lobbyid;}
		HelloFromPeer(uint8_t peerid,int32_t lobbyid){
			m_peerid = peerid;
			m_lobbyid = lobbyid;
		}
	};
	
	HelloFromPeer * RECEIVE_HELLOFROMPEER(BYTEARRAY data);

	class ChatSend
	{
	private:
		string m_signature;
		string m_message;
	public:
		string GetSignature(){return m_signature;}
		string GetMessage(){return m_message;}
		ChatSend(string signature, string message){
			m_signature = signature;
			m_message = message;
		}
	};

	ChatSend * RECEIVE_CHATSEND(BYTEARRAY data);
		
	class Packet
	{
	private:
		int m_ID;
		BYTEARRAY m_Data;
		uint16_t m_Length;
	public:
		Packet( uint16_t nLength, int nID, BYTEARRAY nData )
		{
			m_Length = nLength;
			m_ID = nID;
			m_Data = nData;
		}
		int GetID( )					{ return m_ID; }
		uint16_t GetLength( )			{ return m_Length; }
		BYTEARRAY GetData( )			{ return m_Data; }
	};
	//Other functions
	bool ExtractPacket( BYTEARRAY Bytes, Packet ** packet);

	DGMTP2PProtocol(bool bigEndian);
private:

	bool m_isBigEndian;
	CryptoPP::AutoSeededRandomPool m_rng;
	bool AssignLength( BYTEARRAY &content );
	bool ValidateLength( BYTEARRAY &content );
	bool AppendHeader( BYTEARRAY &content );

};


#endif