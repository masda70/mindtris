
#include "includes.h"
#include "util.h"
#include "serverprotocol.h"

///////////////////////
// SEND FUNCTIONS    //
///////////////////////

BYTEARRAY DGMTProtocol::SEND_DGMT_LEAVELOBBY()
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMT_LEAVELOBBY );			// DGMT_LEAVELOBBY
	AssignLength(packet);
	return packet;
}

BYTEARRAY DGMTProtocol::SEND_DGMT_JOINLOBBY(uint32_t lobbyid, string password, uint16_t portnumber,RSAPublicKey * publickey)
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMT_JOINLOBBY );			// DGMT_JOINLOBBY
	
	UTIL_AppendByteArray(packet, (uint32_t) lobbyid, m_isBigEndian);

	UTIL_AppendByteArray(packet, (uint8_t) password.length(),m_isBigEndian);
	UTIL_AppendByteArray(packet, password, false);

	UTIL_AppendByteArray(packet, portnumber,m_isBigEndian);

	UTIL_AppendByteArray(packet, (uint16_t) publickey->Modulus.length(),m_isBigEndian);
	UTIL_AppendByteArray(packet, publickey->Modulus, false);

	UTIL_AppendByteArray(packet, (uint8_t) publickey->Exponent.length(),m_isBigEndian);
	UTIL_AppendByteArray(packet, publickey->Exponent, false);

	AssignLength( packet );
	return packet;
}
BYTEARRAY DGMTProtocol::SEND_DGMT_JOINEDLOBBY(uint32_t lobbyid, JoinedLobby answer)
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMT_JOINEDLOBBY );			// DGMT_JOINEDLOBBY
	
	UTIL_AppendByteArray(packet, lobbyid, m_isBigEndian);  // push, MMM
	packet.push_back( answer);
		AssignLength( packet );
	return packet;
}
BYTEARRAY DGMTProtocol::SEND_DGMT_JOINEDLOBBY(uint32_t lobbyid, JoinedLobby answer, uint8_t peerid, uint64_t sessionid, vector<DGMTClientLobbyInfo> * clientinfolist)
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMT_JOINEDLOBBY );			// DGMT_JOINEDLOBBY
	
	UTIL_AppendByteArray(packet, lobbyid, m_isBigEndian);  // push, MMM
	packet.push_back( answer);

	UTIL_AppendByteArray(packet, peerid, m_isBigEndian); // proxy gate

	UTIL_AppendByteArray(packet, sessionid, m_isBigEndian); // 6 pool

	packet.push_back((uint8_t) clientinfolist->size());

	for( vector<DGMTClientLobbyInfo> :: iterator v = clientinfolist->begin( ); v != clientinfolist->end( ); v++  )
	{
		UTIL_AppendByteArray(packet, v->GetPeerID(), m_isBigEndian);

		UTIL_AppendByteArray(packet, (uint8_t) v->GetDisplayName().length(), m_isBigEndian);
		UTIL_AppendByteArray(packet, v->GetDisplayName(), false);

		UTIL_AppendByteArray(packet, v->GetIPAddress(), m_isBigEndian);
		UTIL_AppendByteArray(packet, v->GetPortNumber(), m_isBigEndian);

		RSAPublicKey * publickey = v->GetPublicKey();
		UTIL_AppendByteArray(packet, (uint16_t) publickey->Modulus.length(),m_isBigEndian);
		UTIL_AppendByteArray(packet, publickey->Modulus, false);

		UTIL_AppendByteArray(packet, (uint8_t) publickey->Exponent.length(),m_isBigEndian);
		UTIL_AppendByteArray(packet, publickey->Exponent, false);

	}

	AssignLength( packet );
	return packet;
}

BYTEARRAY DGMTProtocol::SEND_DGMT_HELLOFROMCLIENT()
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMT_HELLOFROMCLIENT );	// DGMT_HELLOFROMCLIENT
	UTIL_AppendByteArray(packet,DGMTProtocol::DGMT_PROTOCOLVERSION,m_isBigEndian);
	AssignLength( packet );

	return packet;
}

BYTEARRAY DGMTProtocol::SEND_DGMT_HELLOFROMSERVER(ServerHello answer, RSAPublicKey * publickey, string Message)
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMT_HELLOFROMSERVER );	// HELLO FROM SERVER
	packet.push_back( answer );	// server reply

	UTIL_AppendByteArray(packet, (uint16_t) publickey->Modulus.length(),m_isBigEndian);
	UTIL_AppendByteArray(packet, publickey->Modulus, false);

	UTIL_AppendByteArray(packet, (uint8_t) publickey->Exponent.length(),m_isBigEndian);
	UTIL_AppendByteArray(packet, publickey->Exponent, false);

	UTIL_AppendByteArray(packet, (uint16_t) Message.length(),m_isBigEndian);
	UTIL_AppendByteArray(packet, Message, false);

	AssignLength( packet );

	return packet;
}


BYTEARRAY DGMTProtocol::SEND_DGMT_KEEPALIVE()
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	AssignLength( packet );
	return packet;
}


BYTEARRAY DGMTProtocol::SEND_DGMT_USERCREATION(UserCreation answer)
{

	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMT_USERCREATION );		// DGMT_CREATEUSER
	packet.push_back( answer );					// server reply

	AssignLength( packet );
	return packet;
}


BYTEARRAY DGMTProtocol::SEND_DGMT_CREATEUSER(string username, string display_name, string email, string password, CryptoPP::PK_Encryptor * encryptor)
{

	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMT_CREATEUSER );		// DGMT_CREATEUSER

	UTIL_AppendByteArray(packet,(uint8_t) username.length(),m_isBigEndian);
	UTIL_AppendByteArray(packet,username,false);

	UTIL_AppendByteArray(packet,(uint8_t) display_name.length(),m_isBigEndian);
	UTIL_AppendByteArray(packet,display_name,false);

	UTIL_AppendByteArray(packet,(uint16_t) email.length(),m_isBigEndian);
	UTIL_AppendByteArray(packet,email,false);

	string encrypted;

	CryptoPP::StringSource( password, true,
		new CryptoPP::PK_EncryptorFilter(m_rng, *encryptor,
			new CryptoPP::StringSink( encrypted )
		) // PK_EncryptorFilter
	 ); // StringSource

	UTIL_AppendByteArray(packet,(uint16_t) encrypted.length(),m_isBigEndian);
	UTIL_AppendByteArray(packet,encrypted,false);

	AssignLength( packet );
	return packet;
}


BYTEARRAY DGMTProtocol::SEND_DGMT_LOGIN(string username, string password, CryptoPP::PK_Encryptor * encryptor)
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMT_LOGIN );		// DGMT_LOGIN

	UTIL_AppendByteArray(packet,(uint8_t) username.length(),m_isBigEndian);
	UTIL_AppendByteArray(packet,username,false);

	string encrypted;

	CryptoPP::StringSource( password, true,
		new CryptoPP::PK_EncryptorFilter(m_rng, *encryptor,
			new CryptoPP::StringSink( encrypted )
		) // PK_EncryptorFilter
	 ); // StringSource

	UTIL_AppendByteArray(packet,(uint16_t) encrypted.length(),m_isBigEndian);
	UTIL_AppendByteArray(packet,encrypted,false);

	AssignLength( packet );
	return packet;
}


BYTEARRAY DGMTProtocol::SEND_DGMT_LOGINREPLY(LoginReply reply, string display_name)
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMT_LOGINREPLY );				// DGMT_LOGINREPLY

	packet.push_back( reply );

	if(reply == LOGINREPLY_SUCCESS || reply == LOGINREPLY_SUCCESSDISCONNECTEDELSEWHERE)
	{
		UTIL_AppendByteArray(packet,(uint8_t) display_name.length(),m_isBigEndian);
		UTIL_AppendByteArray(packet,display_name,false);
	}

	AssignLength( packet );
	return packet;
}

BYTEARRAY DGMTProtocol::SEND_DGMT_CREATELOBBY(string lobbyname, uint8_t maxplayers, bool haspassword, string password,  CryptoPP::PK_Encryptor * encryptor )
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMT_CREATELOBBY );		// DGMT_CREATELOBBY

	UTIL_AppendByteArray(packet,(uint8_t) lobbyname.length(),m_isBigEndian);
	UTIL_AppendByteArray(packet,lobbyname,false);

	UTIL_AppendByteArray(packet,maxplayers,m_isBigEndian);

	packet.push_back( haspassword?1:0 );

	string encrypted;

	CryptoPP::StringSource( password, true,
		new CryptoPP::PK_EncryptorFilter(m_rng, *encryptor,
			new CryptoPP::StringSink( encrypted )
		) // PK_EncryptorFilter
	 ); // StringSource

	UTIL_AppendByteArray(packet,(uint16_t) encrypted.length(),m_isBigEndian);
	UTIL_AppendByteArray(packet,encrypted,false);

	AssignLength( packet );
	return packet;
}

BYTEARRAY DGMTProtocol::SEND_DGMT_LOBBYCREATION(LobbyCreation answer, uint32_t lobbyid, uint64_t sessionid)
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMT_LOBBYCREATION );		// DGMT_LOBBYCREATION

	packet.push_back(answer);
	UTIL_AppendByteArray(packet,lobbyid,m_isBigEndian);
	UTIL_AppendByteArray(packet,sessionid,m_isBigEndian);

	AssignLength( packet );
	return packet;

}

BYTEARRAY DGMTProtocol::SEND_DGMT_UPDATECLIENTSTATUS(StatusUpdate answer, uint8_t peerid)
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMT_UPDATECLIENTSTATUS );		// DGMT_UPDATECLIENTSTATUS

	packet.push_back(answer);

	UTIL_AppendByteArray(packet,peerid,m_isBigEndian);

	AssignLength( packet );
	return packet;
}
BYTEARRAY DGMTProtocol::SEND_DGMT_UPDATECLIENTSTATUS(StatusUpdate answer, uint8_t peerid, string DisplayName, uint32_t ipaddress, uint16_t portnumber, RSAPublicKey * publickey)
{
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMT_UPDATECLIENTSTATUS );		// DGMT_UPDATECLIENTSTATUS

	packet.push_back(answer);

	UTIL_AppendByteArray(packet,peerid,m_isBigEndian);

	UTIL_AppendByteArray(packet,(uint8_t) DisplayName.length(),m_isBigEndian);
	UTIL_AppendByteArray(packet,DisplayName,false);

	UTIL_AppendByteArray(packet,(uint32_t) ipaddress,m_isBigEndian);
	UTIL_AppendByteArray(packet,(uint16_t) portnumber,m_isBigEndian);


	UTIL_AppendByteArray(packet, (uint16_t) publickey->Modulus.length(),m_isBigEndian);
	UTIL_AppendByteArray(packet, publickey->Modulus, false);

	UTIL_AppendByteArray(packet, (uint8_t) publickey->Exponent.length(),m_isBigEndian);
	UTIL_AppendByteArray(packet, publickey->Exponent, false);

	AssignLength( packet );
	return packet;
}

BYTEARRAY DGMTProtocol::SEND_DGMT_GETLOBBYLIST(){
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMT_GETLOBBYLIST);		// DGMT_GETLOBBYLIST

	AssignLength( packet );
	return packet;
}

BYTEARRAY DGMTProtocol::SEND_DGMT_LOBBYLIST(vector<DGMTLobbyInfo> * infolist){
	BYTEARRAY packet;
	AppendHeader( packet );			        	// DGMT header constant

	packet.push_back( DGMT_LOBBYLIST);		// DGMT_LOBBYLIST

	packet.push_back((uint8_t) infolist->size());

	for( vector<DGMTLobbyInfo> :: iterator v = infolist->begin( ); v != infolist->end( ); v++  )
	{
		UTIL_AppendByteArray(packet, v->GetLobbyId(), m_isBigEndian);

		UTIL_AppendByteArray(packet, (uint8_t) v->GetLobbyName().length(), m_isBigEndian);
		UTIL_AppendByteArray(packet, v->GetLobbyName(), false);

		UTIL_AppendByteArray(packet, v->GetPlayerCount(), m_isBigEndian);
		UTIL_AppendByteArray(packet, v->GetMaxPlayers(), m_isBigEndian);

		packet.push_back(v->GetHasPassword()?0x01:0x00);

		UTIL_AppendByteArray(packet, (uint8_t) v->GetCreatorDisplayName().length(), m_isBigEndian);
		UTIL_AppendByteArray(packet, v->GetCreatorDisplayName(), false);
	}

	AssignLength( packet );
	return packet;
}

///////////////////////
// RECEIVE FUNCTIONS //
///////////////////////


vector<DGMTLobbyInfo> * DGMTProtocol::RECEIVE_DGMT_LOBBYLIST(BYTEARRAY data){
	int offset = 0;
	int length;
	int datasize = data.size( );

		length = 1;
		if(!(datasize >= length + offset)) return NULL;
		uint8_t lobbylist_size = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
		offset += length;

		vector<DGMTLobbyInfo> * lobbylist = new vector<DGMTLobbyInfo>;
		lobbylist->reserve(lobbylist_size);

		for(int i = 1; i<=lobbylist_size ;i++)
		{

			length = 4;
			if(!(datasize >= length + offset)) return NULL;
			uint32_t lobbyid = UTIL_ByteArrayToUInt32( data, m_isBigEndian, offset );
			offset += length;

			length = 1;
			if(!(datasize >= length + offset)) return NULL;
			uint8_t lobbyname_length = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
			offset += length;

			length = lobbyname_length;
			if(!(datasize >= length + offset)) return NULL;
			string lobbyname = string( data.begin( ) + offset, data.begin( ) + offset + length );
			offset += length;
			
			length = 1;
			if(!(datasize >= length + offset)) return NULL;
			uint8_t playercount = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
			offset += length;

			length = 1;
			if(!(datasize >= length + offset)) return NULL;
			uint8_t maxplayers = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
			offset += length;

			length = 1;
			if(!(datasize >= length + offset)) return NULL;
			bool haspassword = (data[offset] == 0x00)?false: true;
			offset += length;

			length = 1;
			if(!(datasize >= length + offset)) return NULL;
			uint8_t creatorname_length = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
			offset += length;

			length = creatorname_length;
			if(!(datasize >= length + offset)) return NULL;
			string creatorname = string( data.begin( ) + offset, data.begin( ) + offset + length );
			offset += length;

			lobbylist->push_back(DGMTLobbyInfo(lobbyid,lobbyname,playercount,maxplayers,haspassword,creatorname));
		}

		return lobbylist;

}


void DGMTProtocol::RECEIVE_DGMT_GETLOBBYLIST(BYTEARRAY data){
	return;
}

DGMTUpdateClientStatus * DGMTProtocol::RECEIVE_DGMT_UPDATECLIENTSTATUS(BYTEARRAY data)
{
	int offset = 0;
	int length;
	int datasize = data.size( );

		length = 1;
		if(!(datasize >= length + offset)) return NULL;
		StatusUpdate statusupdate = (StatusUpdate) data[offset];
		offset += length;

		length = 1;
		if(!(datasize >= length + offset)) return NULL;
		uint8_t peerid = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
		offset += length;

		if(statusupdate != STATUSUPDATE_HASJOINEDTHELOBBY) return (new DGMTUpdateClientStatus(statusupdate, new DGMTClientLobbyInfo(peerid)));

		length = 1;
		if(!(datasize >= length + offset)) return NULL;
		uint8_t displayname_length = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
		offset += length;

		length = displayname_length;
		if(!(datasize >= length + offset)) return NULL;
		string displayname = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		length = 4;
		if(!(datasize >= length + offset)) return NULL;
		uint32_t ipaddress = UTIL_ByteArrayToUInt32( data, m_isBigEndian, offset );
		offset += length;

		length = 2;
		if(!(datasize >= length + offset)) return NULL;
		uint16_t portnumber = UTIL_ByteArrayToUInt16( data, m_isBigEndian, offset );
		offset += length;

		length = 2;
		if(!(datasize >= length + offset)) return NULL;
		uint16_t modulus_length = UTIL_ByteArrayToUInt16( data, m_isBigEndian, offset );
		offset += length;

		length = modulus_length;
		if(!(datasize >= length + offset)) return NULL;
		string modulus = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		length = 1;
		if(!(datasize >= length + offset)) return NULL;
		uint8_t exponent_length = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
		offset += length;

		length = exponent_length;
		if(!(datasize >= length + offset)) return NULL;
		string exponent = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		RSAPublicKey * key = new RSAPublicKey();
		key->Exponent = exponent;
		key->Modulus = modulus;

		 return (new DGMTUpdateClientStatus(statusupdate, new DGMTClientLobbyInfo(peerid,displayname,ipaddress,portnumber,key)));

}

void DGMTProtocol::RECEIVE_DGMT_LEAVELOBBY(BYTEARRAY data)
{
	return;
}


DGMTJoinLobby * DGMTProtocol::RECEIVE_DGMT_JOINLOBBY(BYTEARRAY data)
{
	int offset = 0;
	int length;
	int datasize = data.size( );

		length = 4;
		if(!( datasize >= length + offset)) return NULL;
		uint32_t lobbyid = UTIL_ByteArrayToUInt32( data, m_isBigEndian, offset );
		offset += length;
		
		length = 1;
		if(!( datasize >= length + offset)) return NULL;
		uint8_t password_length = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
		offset += length;
		
		length = password_length;
		if(!(datasize >= length + offset)) return NULL;
		string password = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		length = 2;
		if(!( datasize >= length + offset)) return NULL;
		uint16_t portnumber = UTIL_ByteArrayToUInt16( data, m_isBigEndian, offset );
		offset += length;

		length = 2;
		if(!(datasize >= length + offset)) return NULL;
		uint16_t modulus_length = UTIL_ByteArrayToUInt16( data, m_isBigEndian, offset );
		offset += length;

		length = modulus_length;
		if(!(datasize >= length + offset)) return NULL;
		string modulus = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		length = 1;
		if(!(datasize >= length + offset)) return NULL;
		uint8_t exponent_length = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
		offset += length;

		length = exponent_length;
		if(!(datasize >= length + offset)) return NULL;
		string exponent = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		RSAPublicKey * key = new RSAPublicKey();
		key->Exponent = exponent;
		key->Modulus = modulus;

		return new DGMTJoinLobby(lobbyid,password,portnumber,key);

}

DGMTJoinedLobby * DGMTProtocol::RECEIVE_DGMT_JOINEDLOBBY(BYTEARRAY data)
{
	int offset = 0;
	int length;
	int datasize = data.size( );

		length = 4;
		if(!( datasize >= length + offset)) return NULL;
		uint32_t lobbyid = UTIL_ByteArrayToUInt32( data, m_isBigEndian, offset );
		offset += length;
		
		length = 1;
		if(!( datasize >= length + offset)) return NULL;
		JoinedLobby answer = (JoinedLobby) data[offset];
		offset += length;

		if(answer != JOINEDLOBBY_SUCCESS) return new DGMTJoinedLobby(lobbyid,answer);
		
		length = 1;
		if(!(datasize >= length + offset)) return NULL;
		uint8_t peerid = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
		offset += length;

		length = 8;
		if(!(datasize >= length + offset)) return NULL;
		uint64_t sessionid = UTIL_ByteArrayToUInt64( data, m_isBigEndian, offset );
		offset += length;

		length = 1;
		if(!(datasize >= length + offset)) return NULL;
		uint8_t clientlobbylist_size = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
		offset += length;

		vector<DGMTClientLobbyInfo> * clientlobbylist = new vector<DGMTClientLobbyInfo>;
		clientlobbylist->reserve(clientlobbylist_size);

		for(int i = 1; i<=clientlobbylist_size ;i++)
		{
			length = 1;
			if(!(datasize >= length + offset)) return NULL;
			uint8_t peerid = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
			offset += length;

			length = 1;
			if(!(datasize >= length + offset)) return NULL;
			uint8_t displayname_length = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
			offset += length;

			length = displayname_length;
			if(!(datasize >= length + offset)) return NULL;
			string displayname = string( data.begin( ) + offset, data.begin( ) + offset + length );
			offset += length;

			length = 4;
			if(!(datasize >= length + offset)) return NULL;
			uint32_t ipaddress = UTIL_ByteArrayToUInt32( data, m_isBigEndian, offset );
			offset += length;

			length = 2;
			if(!(datasize >= length + offset)) return NULL;
			uint16_t portnumber = UTIL_ByteArrayToUInt16( data, m_isBigEndian, offset );
			offset += length;

			length = 2;
			if(!(datasize >= length + offset)) return NULL;
			uint16_t modulus_length = UTIL_ByteArrayToUInt16( data, m_isBigEndian, offset );
			offset += length;

			length = modulus_length;
			if(!(datasize >= length + offset)) return NULL;
			string modulus = string( data.begin( ) + offset, data.begin( ) + offset + length );
			offset += length;

			length = 1;
			if(!(datasize >= length + offset)) return NULL;
			uint8_t exponent_length = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
			offset += length;

			length = exponent_length;
			if(!(datasize >= length + offset)) return NULL;
			string exponent = string( data.begin( ) + offset, data.begin( ) + offset + length );
			offset += length;

			RSAPublicKey * key = new RSAPublicKey();
			key->Exponent = exponent;
			key->Modulus = modulus;

			 clientlobbylist->push_back(DGMTClientLobbyInfo(peerid,displayname,ipaddress,portnumber,key));
		}

		return new DGMTJoinedLobby(lobbyid,answer,peerid,sessionid,clientlobbylist);

}


DGMTCreateLobby * DGMTProtocol::RECEIVE_DGMT_CREATELOBBY(BYTEARRAY data,  CryptoPP::PK_Decryptor * Decryptor )
{
	int offset = 0;
	int length;
	int datasize = data.size( );

		length = 1;
		if(!( datasize >= length + offset)) return NULL;
		uint8_t lobbyname_length = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
		offset += length;

		length = lobbyname_length;
		if(!(datasize >= length + offset)) return NULL;
		string lobbyname = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		length = 1;
		if(!( datasize >= length + offset)) return NULL;
		uint8_t maxplayers = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
		offset += length;

		length = 1;
		if(!( datasize >= length + offset)) return NULL;
		bool haspassword = (data[offset]==0x00)?false:true;
		offset += length;

		length = 2;
		if(!( datasize >= length + offset)) return NULL;
		uint16_t encryptedpassword_length = UTIL_ByteArrayToUInt16( data, m_isBigEndian, offset );
		offset += length;

		length = encryptedpassword_length;
		if(!(datasize >= length + offset)) return NULL;
		string encryptedpassword = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		string password;
		try{
			CryptoPP::StringSource( encryptedpassword, true,
			  new CryptoPP::PK_DecryptorFilter( m_rng, *Decryptor,
				new CryptoPP::StringSink( password )
				) // PK_DecryptorFilter
			); // StringSource
		} catch (int e){
			cout << "An exception occurred. Exception Nr. " << e << endl;
			return NULL;
		}

		return new DGMTCreateLobby(lobbyname,maxplayers,haspassword,password);

}

DGMTLobbyCreation * DGMTProtocol::RECEIVE_DGMT_LOBBYCREATION(BYTEARRAY data)
{
	int offset = 0;
	int length;
	int datasize = data.size( );

		length = 1;
		if(!( datasize >= length + offset)) return NULL;
		LobbyCreation answer = (LobbyCreation) data[offset];
		offset += length;

		length = 4;
		if(!( datasize >= length + offset)) return NULL;
		uint32_t lobbyid = UTIL_ByteArrayToUInt32( data, m_isBigEndian, offset );
		offset += length;

		length = 8;
		if(!( datasize >= length + offset)) return NULL;
		uint64_t sessionid = UTIL_ByteArrayToUInt64( data, m_isBigEndian, offset );
		offset += length;

		return new DGMTLobbyCreation(answer,lobbyid,sessionid);

}

DGMTLoginReply * DGMTProtocol::RECEIVE_DGMT_LOGINREPLY(BYTEARRAY data)
{
	int offset = 0;
	int length;
	int datasize = data.size( );

		length = 1;
		if(!( datasize >= length + offset)) return NULL;
		LoginReply answer = (LoginReply) data[offset];
		offset += length;

		if(answer!=LOGINREPLY_SUCCESS && answer !=LOGINREPLY_SUCCESSDISCONNECTEDELSEWHERE) return new DGMTLoginReply(answer,"");
		length = 1;
		if(!( datasize >= length + offset)) return NULL;
		uint8_t displayname_length = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
		offset += length;

		length = displayname_length;
		if(!(datasize >= length + offset)) return NULL;
		string displayname = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		return new DGMTLoginReply(answer,displayname);

}

DGMTLoginInfo * DGMTProtocol::RECEIVE_DGMT_LOGIN(BYTEARRAY data, CryptoPP::PK_Decryptor * Decryptor )
{
	int offset = 0;
	int length;
	int datasize = data.size( );

		length = 1;
		if(!( datasize >= length + offset)) return NULL;
		uint8_t username_length = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
		offset += length;

		length = username_length;
		if(!(datasize >= length + offset)) return NULL;
		string username = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		length = 2;
		if(!( datasize >= length + offset)) return NULL;
		uint16_t encryptedpassword_length = UTIL_ByteArrayToUInt16( data, m_isBigEndian, offset );
		offset += length;

		length = encryptedpassword_length;
		if(!(datasize >= length + offset)) return NULL;
		string encryptedpassword = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		string password;
		try{
			CryptoPP::StringSource( encryptedpassword, true,
			  new CryptoPP::PK_DecryptorFilter( m_rng, *Decryptor,
				new CryptoPP::StringSink( password )
				) // PK_DecryptorFilter
			); // StringSource
		}catch (int e){
			cout << "An exception occurred. Exception Nr. " << e << endl;
			return NULL;
		}


		return new DGMTLoginInfo(username,password);

}

DGMTProtocol::UserCreation DGMTProtocol::RECEIVE_DGMT_USERCREATION(BYTEARRAY data)
{
	if(data.size() >=1)
	{
		return (UserCreation) data[0];
	}
	return USERCREATION_INVALIDPACKET;
}

DGMTCreateUserInfo * DGMTProtocol::RECEIVE_DGMT_CREATEUSER( BYTEARRAY data, CryptoPP::PK_Decryptor * Decryptor){
	int offset = 0;
	int length;
	int datasize = data.size( );

		length = 1;
		if(!( datasize >= length + offset)) return NULL;
		uint8_t username_length = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
		offset += length;

		length = username_length;
		if(!(datasize >= length + offset)) return NULL;
		string username = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		length = 1;
		if(!( datasize >= length + offset)) return NULL;
		uint8_t displayname_length = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
		offset += length;

		length = displayname_length;
		if(!( datasize >= length + offset)) return NULL;
		string displayname = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		length = 2;
		if(!( datasize >= length + offset)) return NULL;
		uint16_t email_length = UTIL_ByteArrayToUInt16( data, m_isBigEndian, offset );
		offset += length;

		length = email_length;
		if(!( datasize >= length + offset)) return NULL;
		string email = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		length = 2;
		if(!( datasize >= length + offset)) return NULL;
		uint16_t encryptedpassword_length = UTIL_ByteArrayToUInt16( data, m_isBigEndian, offset );
		offset += length;

		length = encryptedpassword_length;
		if(!(datasize >= length + offset)) return NULL;
		string encryptedpassword = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		string password;
		try{

			CryptoPP::StringSource( encryptedpassword, true,
			  new CryptoPP::PK_DecryptorFilter( m_rng, *Decryptor,
				new CryptoPP::StringSink( password )
				) // PK_DecryptorFilter
			); // StringSource
		}catch ( int e)
		{
			cout << "An exception occurred. Exception Nr. " << e << endl;
			return NULL;
		}


		return new DGMTCreateUserInfo(username,displayname,email,password);

}

uint32_t DGMTProtocol::RECEIVE_DGMT_HELLOFROMCLIENT(BYTEARRAY data)
{
	if(data.size() >= 4)
	{
		return UTIL_ByteArrayToUInt32( data, m_isBigEndian, 0 );
	}
	return (uint32_t) 0;
}


DGMTConnectionReply * DGMTProtocol::RECEIVE_DGMT_HELLOFROMSERVER(BYTEARRAY data)
{
	int offset = 0;
	int length;
	int datasize = data.size( );

	length = 1;
	if(!( datasize >= length + offset)) return NULL;
	ServerHello answer = (ServerHello) data[offset];
	offset += length;

		
		length = 2;
		if(!(datasize >= length + offset)) return NULL;
		uint16_t modulus_length = UTIL_ByteArrayToUInt16( data, m_isBigEndian, offset );
		offset += length;

		length = modulus_length;
		if(!(datasize >= length + offset)) return NULL;
		string modulus = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		length = 1;
		if(!(datasize >= length + offset)) return NULL;
		uint8_t exponent_length = UTIL_ByteArrayToUInt8( data, m_isBigEndian, offset );
		offset += length;

		length = exponent_length;
		if(!(datasize >= length + offset)) return NULL;
		string exponent = string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		RSAPublicKey * key = new RSAPublicKey();
		key->Exponent = exponent;
		key->Modulus = modulus;

		length = 2;
		if(!( datasize >= length + offset)) return NULL;
		uint16_t message_length = UTIL_ByteArrayToUInt16( data, m_isBigEndian, offset );
		offset += length;

		length = message_length;
		if(!( datasize >= length + offset)) return NULL;
		string message= string( data.begin( ) + offset, data.begin( ) + offset + length );
		offset += length;

		if(answer == SHELLO_CONNECTED)
		{
			return new DGMTConnectionReply(true, key, message);
		}else
		{
			if(answer == SHELLO_REFUSEDWRONGVERSION)
			{
				return new DGMTConnectionReply(false, key, "Server refused connection. Wrong client version:"+message);
			}else{
				return new DGMTConnectionReply(false, key, message);
			}
		}

}



/////////////////////
// OTHER FUNCTIONS //
/////////////////////

bool DGMTProtocol :: ExtractPacket( BYTEARRAY Bytes, DGMTPacket ** packet)
{
	if (Bytes[0] == DGMT_HEADER_CONSTANT1 && Bytes[1] == DGMT_HEADER_CONSTANT2 && Bytes[2] == DGMT_HEADER_CONSTANT3 && Bytes[3] == DGMT_HEADER_CONSTANT4){
		uint16_t Length  = UTIL_ByteArrayToUInt16( Bytes, m_isBigEndian, 4 );
		if(Length >= HEADERLENGTH)
		{
			if(Length <= Bytes.size()){
				if(Length == HEADERLENGTH)
				{
					*packet = new DGMTPacket(Length, DGMT_KEEPALIVE, BYTEARRAY( Bytes.begin( ) + HEADERLENGTH+1, Bytes.begin( ) + Length ));
				}else{
					*packet = new DGMTPacket(Length, Bytes[HEADERLENGTH], BYTEARRAY( Bytes.begin( ) + HEADERLENGTH+1, Bytes.begin( ) + Length ));
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


bool DGMTProtocol :: AppendHeader( BYTEARRAY &content)
{
	content.push_back(DGMT_HEADER_CONSTANT1);
	content.push_back(DGMT_HEADER_CONSTANT2);
	content.push_back(DGMT_HEADER_CONSTANT3);
	content.push_back(DGMT_HEADER_CONSTANT4);
	content.push_back( 0 );						// packet length will be assigned later
	content.push_back( 0 );						// packet length will be assigned later
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




DGMTPacket:: DGMTPacket( uint16_t nLength, int nID, BYTEARRAY nData )
{
	m_Length = nLength;
	m_ID = nID;
	m_Data = nData;
}