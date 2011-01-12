
#include "includes.h"
#include "util.h"
#include "bytearray.h"
#include "commprotocol.h"
#include "serverprotocol.h"


const ByteArray & DGMTProtocol::GetProtocolIdentifier() const{
	static ByteArray s_protocolidentifier(DGMT_PROTOCOLIDENTIFIER);
	return s_protocolidentifier;
};

const bool DGMTProtocol::IsBigEndian() const{
	static bool s_isbigendian = true;
	return s_isbigendian;
}

DGMTProtocol::DGMTProtocol(): m_parser(IsBigEndian()){}

///////////////////////
// SEND FUNCTIONS    //
///////////////////////



ByteArray DGMTProtocol::SEND_LEAVELOBBY()
{
	MessageBuilder builder(IsBigEndian());
	builder.append_BYTE((byte_t)TYPE_LEAVELOBBY );			// LEAVELOBBY

	return builder.GetResult();
}


ByteArray DGMTProtocol::SEND_JOINLOBBY(uint32_t lobbyid, const string & password, uint16_t portnumber,const DSAPublicKey & publickey)
{
	MessageBuilder builder(IsBigEndian());

	builder.append_BYTE( TYPE_JOINLOBBY );			// JOINLOBBY

	builder.append_INTEGER32( lobbyid );
	builder.append_STRING8( password );
	builder.append_INTEGER16( portnumber );
	builder.append_DSAPublicKey( publickey);

	return builder.GetResult();
}


ByteArray DGMTProtocol::SEND_JOINEDLOBBY(uint32_t lobbyid, DGMTProtocol::JoinedLobby::answer answer)
{
	MessageBuilder builder(IsBigEndian());

	builder.append_BYTE((byte_t) TYPE_JOINEDLOBBY );			// JOINLOBBY

	builder.append_INTEGER32(lobbyid );	
	builder.append_BYTE( answer);

	return builder.GetResult();
}

ByteArray DGMTProtocol::SEND_JOINEDLOBBY(uint32_t lobbyid, DGMTProtocol::JoinedLobby::answer answer, const string &lobbyname, uint8_t maxplayers, uint8_t creatorid, uint8_t peerid, uint64_t sessionid, const vector<DGMTProtocol::ClientLobbyInfo> & clientinfolist)
{
	MessageBuilder builder(IsBigEndian());

	builder.append_BYTE((byte_t) TYPE_JOINEDLOBBY );	

	builder.append_INTEGER32(lobbyid );		
	builder.append_BYTE( answer);
	builder.append_USTRING8(lobbyname);
	builder.append_INTEGER8(maxplayers);
	builder.append_INTEGER8(creatorid);
	builder.append_INTEGER8(peerid);
	builder.append_INTEGER64(sessionid);
	builder.append_INTEGER8((uint8_t) clientinfolist.size());

	for( vector<DGMTProtocol::ClientLobbyInfo> :: const_iterator v = clientinfolist.begin( ); v != clientinfolist.end( ); v++  )
	{
		builder.append_INTEGER8(v->GetPeerID());
		builder.append_USTRING8(v->GetDisplayName());
		builder.append_INTEGER32(v->GetIPAddress());
		builder.append_INTEGER16(v->GetPortNumber());
		builder.append_DSAPublicKey(v->GetPublicKey());
	}

	return builder.GetResult();

}

ByteArray DGMTProtocol::SEND_HELLOFROMCLIENT()
{
	MessageBuilder builder(IsBigEndian());

	builder.append_BYTE( TYPE_HELLOFROMCLIENT );			

	builder.append_INTEGER32( DGMTProtocol::PROTOCOLVERSION);

	return builder.GetResult();
}

ByteArray DGMTProtocol::SEND_HELLOFROMSERVER(ServerHello answer, const RSAPublicKey & publickey, const string & Message)
{
	MessageBuilder builder(IsBigEndian());

	builder.append_BYTE( (byte_t) TYPE_HELLOFROMSERVER );			

	builder.append_BYTE( (byte_t) answer );		

	builder.append_RSAPublicKey(publickey);
	builder.append_USTRING16(Message);

	return builder.GetResult();
}


ByteArray DGMTProtocol::SEND_KEEPALIVE()
{
	return ByteArray();
}


ByteArray DGMTProtocol::SEND_USERCREATION(UserCreation answer)
{
	MessageBuilder builder(IsBigEndian());

	builder.append_BYTE( (byte_t)TYPE_USERCREATION );			
	builder.append_BYTE( answer );

	return builder.GetResult();
}


ByteArray DGMTProtocol::SEND_CREATEUSER(const string &username, const string &display_name, const string &email, const string &password, const CryptoPP::PK_Encryptor & encryptor)
{

	MessageBuilder builder(IsBigEndian());

	builder.append_BYTE( TYPE_CREATEUSER );			

	builder.append_USTRING8(username);
	builder.append_USTRING8(display_name);
	builder.append_STRING16(email);

	string encrypted;

	CryptoPP::StringSource( password, true,
		new CryptoPP::PK_EncryptorFilter(m_rng, encryptor,
			new CryptoPP::StringSink( encrypted )
		) // PK_EncryptorFilter
	 ); // StringSource
	
	builder.append_STRING16(encrypted);

	return builder.GetResult();
}


ByteArray DGMTProtocol::SEND_LOGIN(const string & username, const string & password, const CryptoPP::PK_Encryptor & encryptor)
{
	MessageBuilder builder(IsBigEndian());

	builder.append_BYTE( TYPE_LOGIN );			

	builder.append_USTRING8(username);

	string encrypted;

	CryptoPP::StringSource( password, true,
		new CryptoPP::PK_EncryptorFilter(m_rng, encryptor,
			new CryptoPP::StringSink( encrypted )
		) // PK_EncryptorFilter
	 ); // StringSource

	builder.append_STRING16(encrypted);

	return builder.GetResult();
}


ByteArray DGMTProtocol::SEND_LOGINREPLY( DGMTProtocol::LoginReply::answer reply, const string & display_name)
{	
	MessageBuilder builder(IsBigEndian());

	builder.append_BYTE( (byte_t)TYPE_LOGINREPLY );			

	builder.append_BYTE(reply );

	if(reply == DGMTProtocol::LoginReply::LOGINREPLY_SUCCESS || reply ==  DGMTProtocol::LoginReply::LOGINREPLY_SUCCESSDISCONNECTEDELSEWHERE)
	{
		builder.append_STRING8(display_name);
	}

	return builder.GetResult();
}

ByteArray DGMTProtocol::SEND_CREATELOBBY(const string & lobbyname, uint8_t maxplayers, bool haspassword, const string & password,  const CryptoPP::PK_Encryptor & encryptor,  uint16_t portnumber,const DSAPublicKey & publickey )
{
	MessageBuilder builder(IsBigEndian());
	
	builder.append_BYTE( TYPE_CREATELOBBY );		
	builder.append_STRING8(lobbyname);
	builder.append_INTEGER8(maxplayers);
	builder.append_BOOLEAN(haspassword);
	if(haspassword)
	{
		string encrypted;
		CryptoPP::StringSource( password, true,
			new CryptoPP::PK_EncryptorFilter(m_rng, encryptor,
				new CryptoPP::StringSink( encrypted )
			) // PK_EncryptorFilter
		 ); // StringSource
		builder.append_STRING16(encrypted);
	}

	builder.append_INTEGER16(portnumber);
	builder.append_DSAPublicKey(publickey);

	return builder.GetResult();
}

ByteArray DGMTProtocol::SEND_LOBBYCREATION(DGMTProtocol::LobbyCreation::answer answer)
{
	MessageBuilder builder(IsBigEndian());
	
	builder.append_BYTE((byte_t) TYPE_LOBBYCREATION );	

	builder.append_BYTE(answer);

	return builder.GetResult();

}
ByteArray DGMTProtocol::SEND_LOBBYCREATION(DGMTProtocol::LobbyCreation::answer answer, uint32_t lobbyid, uint8_t peerid, uint64_t sessionid)
{
	MessageBuilder builder(IsBigEndian());
	
	builder.append_BYTE((byte_t) TYPE_LOBBYCREATION );	

	builder.append_BYTE(answer);
	builder.append_INTEGER32(lobbyid);
	builder.append_INTEGER8(peerid);
	builder.append_INTEGER64(sessionid);

	return builder.GetResult();

}

ByteArray DGMTProtocol::SEND_UPDATECLIENTSTATUS(DGMTProtocol::UpdateClientStatus::status answer, uint32_t lobbyid,uint8_t peerid)
{
	MessageBuilder builder(IsBigEndian());
	
	builder.append_BYTE((byte_t) TYPE_UPDATECLIENTSTATUS );	
	
	builder.append_INTEGER32(lobbyid);
	builder.append_BYTE(answer);
	builder.append_INTEGER8(peerid);




	return builder.GetResult();
}
ByteArray DGMTProtocol::SEND_UPDATECLIENTSTATUS(DGMTProtocol::UpdateClientStatus::status answer,  uint32_t lobbyid, uint8_t peerid, const string & DisplayName, uint32_t ipaddress, uint16_t portnumber, const DSAPublicKey & publickey)
{
	MessageBuilder builder(IsBigEndian());
	
	builder.append_BYTE((byte_t) TYPE_UPDATECLIENTSTATUS );	


	builder.append_INTEGER32(lobbyid);
	builder.append_BYTE(answer);
	builder.append_INTEGER8(peerid);

	builder.append_USTRING8(DisplayName);
	builder.append_INTEGER32(ipaddress);
	builder.append_INTEGER16(portnumber);
	builder.append_DSAPublicKey(publickey);

	return builder.GetResult();
}

ByteArray DGMTProtocol::SEND_GETLOBBYLIST(){
	MessageBuilder builder(IsBigEndian());
	
	builder.append_BYTE( TYPE_GETLOBBYLIST );	

	return builder.GetResult();
}

ByteArray DGMTProtocol::SEND_LOBBYLIST(const vector<DGMTProtocol::LobbyInfo> &infolist){
	MessageBuilder builder(IsBigEndian());
	
	builder.append_BYTE((byte_t) TYPE_LOBBYLIST );	

	builder.append_INTEGER8( (uint8_t) infolist.size());

	for( vector<DGMTProtocol::LobbyInfo> :: const_iterator v = infolist.begin( ); v != infolist.end( ); v++  )
	{
		builder.append_INTEGER32( v->GetLobbyId() );
		builder.append_USTRING8( v->GetLobbyName() );
		builder.append_INTEGER8( v->GetPlayerCount() );
		builder.append_INTEGER8( v->GetMaxPlayers() );
		builder.append_BOOLEAN( v->GetHasPassword() );
		builder.append_USTRING8(v->GetCreatorDisplayName());
	}

	return builder.GetResult();
}


ByteArray DGMTProtocol::SEND_STARTGAME(){
	MessageBuilder builder(IsBigEndian());
	
	builder.append_BYTE((byte_t) TYPE_STARTGAME );	

	return builder.GetResult();
}


ByteArray DGMTProtocol::SEND_GAMESTARTING(GameStarting answer){
	MessageBuilder builder(IsBigEndian());
	
	builder.append_BYTE((byte_t) TYPE_GAMESTARTING );	
	builder.append_BYTE((byte_t) answer );	

	return builder.GetResult();

}

ByteArray DGMTProtocol::SEND_LOADGAME(const vector<byte_t> & pieces){
	MessageBuilder builder(IsBigEndian());
	
	builder.append_BYTE((byte_t) TYPE_LOADGAME );	
	builder.append_INTEGER8((uint8_t) pieces.size() );	
	for( vector<uint8_t> :: const_iterator v = pieces.begin( ); v != pieces.end( ); v++  )
	{
		builder.append_BYTE((byte_t) *v);
	}
	return builder.GetResult();

}

ByteArray DGMTProtocol::SEND_LOADEDGAME(LoadedGame answer){
	MessageBuilder builder(IsBigEndian());
	
	builder.append_BYTE((byte_t) TYPE_LOADEDGAME );	
	builder.append_BYTE((byte_t) answer );	
	return builder.GetResult();
}

ByteArray DGMTProtocol::SEND_BEGINGAME(){
	MessageBuilder builder(IsBigEndian());
	
	builder.append_BYTE((byte_t) TYPE_BEGINGAME );	

	return builder.GetResult();
}


ByteArray DGMTProtocol::SEND_NEWPIECES(uint32_t offset, const vector<uint8_t> & pieces){
	MessageBuilder builder(IsBigEndian());
	
	builder.append_BYTE((byte_t) TYPE_NEWPIECES );	
	builder.append_INTEGER32(offset );	
	builder.append_INTEGER8((uint8_t) pieces.size() );	
	for( vector<uint8_t> :: const_iterator v = pieces.begin( ); v != pieces.end( ); v++  )
	{
		builder.append_BYTE((byte_t) *v);
	}
	return builder.GetResult();
}

ByteArray DGMTProtocol::SEND_GIVENEWPIECES(uint32_t offset, uint8_t number){
	MessageBuilder builder(IsBigEndian());
	
	builder.append_BYTE((byte_t) TYPE_GIVENEWPIECES );	
	builder.append_INTEGER32(offset );	
	builder.append_INTEGER8((uint8_t) number );	

	return builder.GetResult();
}


///////////////////////
// RECEIVE FUNCTIONS //
///////////////////////


void DGMTProtocol::RECEIVE_STARTGAME(const ByteArray & message, size_t & offset){
	return;
}

DGMTProtocol::GameStarting DGMTProtocol::RECEIVE_GAMESTARTING(const ByteArray & message, size_t & offset){
	return (GameStarting) m_parser.parse_INTEGER8(message,offset);
}

vector<byte_t> DGMTProtocol::RECEIVE_LOADGAME(const ByteArray & message, size_t & offset){
	vector<byte_t> pieces = vector<byte_t>();
	uint8_t pieces_size = m_parser.parse_INTEGER8(message,offset);
	pieces.reserve(pieces_size);
	for(int i = 1; i<=pieces_size ;i++)
	{
		 pieces.push_back(m_parser.parse_INTEGER8(message,offset));
	}
	return move(pieces);
}

DGMTProtocol::LoadedGame DGMTProtocol::RECEIVE_LOADEDGAME(const ByteArray & message, size_t & offset){
	return (LoadedGame) m_parser.parse_INTEGER8(message,offset);
}

void DGMTProtocol::RECEIVE_BEGINGAME(const ByteArray & message, size_t & offset){
		return;
}

DGMTProtocol::NewPieces DGMTProtocol::RECEIVE_NEWPIECES(const ByteArray & message, size_t & offset){

	uint32_t pieces_offset = m_parser.parse_INTEGER32(message,offset);

	vector<byte_t> pieces = vector<byte_t>();
	uint8_t pieces_size = m_parser.parse_INTEGER8(message,offset);
	pieces.reserve(pieces_size);
	for(int i = 1; i<=pieces_size ;i++)
	{
		 pieces.push_back(m_parser.parse_BYTE(message,offset));
	}

	return NewPieces( pieces_offset,pieces);

}

DGMTProtocol::GiveNewPieces DGMTProtocol::RECEIVE_GIVENEWPIECES(const ByteArray & message, size_t & offset){

	uint32_t pieces_offset = m_parser.parse_INTEGER32(message,offset);
	uint8_t pieces_number = m_parser.parse_INTEGER8(message,offset);

	return GiveNewPieces( pieces_offset,pieces_number);

}


vector<DGMTProtocol::LobbyInfo> DGMTProtocol::RECEIVE_LOBBYLIST(const ByteArray & message, size_t & offset){
	

	uint8_t lobbylist_size = m_parser.parse_INTEGER8(message,offset);

	vector<DGMTProtocol::LobbyInfo> lobbylist = vector<DGMTProtocol::LobbyInfo>();
	lobbylist.reserve(lobbylist_size);

	for(int i = 1; i<=lobbylist_size ;i++)
	{

		uint32_t lobbyid = m_parser.parse_INTEGER32(message,offset);
		string lobbyname = m_parser.parse_USTRING8(message,offset);
		uint8_t playercount = m_parser.parse_INTEGER8(message,offset);
		uint8_t maxplayers = m_parser.parse_INTEGER8(message,offset);
		bool haspassword = m_parser.parse_BOOLEAN(message,offset);
		string creatorname =  m_parser.parse_USTRING8(message,offset);

		lobbylist.push_back(DGMTProtocol::LobbyInfo(lobbyid,lobbyname,playercount,maxplayers,haspassword,creatorname));
	}

	return move(lobbylist);

}


void DGMTProtocol::RECEIVE_GETLOBBYLIST(const ByteArray & message, size_t & offset){
	return;
}

DGMTProtocol::UpdateClientStatus DGMTProtocol::RECEIVE_UPDATECLIENTSTATUS(const ByteArray & message, size_t & offset)
{
	
	uint32_t lobbyid  =m_parser.parse_INTEGER32(message,offset);
	DGMTProtocol::UpdateClientStatus::status statusupdate = (DGMTProtocol::UpdateClientStatus::status) m_parser.parse_BYTE(message,offset);
	uint8_t peerid = m_parser.parse_INTEGER8(message,offset);

	if(statusupdate != DGMTProtocol::UpdateClientStatus::STATUSUPDATE_HASJOINEDTHELOBBY) return DGMTProtocol::UpdateClientStatus(statusupdate,lobbyid, DGMTProtocol::ClientLobbyInfo(peerid));

	string displayname = m_parser.parse_USTRING8(message,offset);
	uint32_t ipaddress =  m_parser.parse_INTEGER32(message,offset);
	uint16_t portnumber = m_parser.parse_INTEGER16(message,offset);
	DSAPublicKey key(m_parser.parse_DSAPublicKey(message,offset));

	return UpdateClientStatus(statusupdate,lobbyid, ClientLobbyInfo(peerid,displayname,ipaddress,portnumber,key));

}

void DGMTProtocol::RECEIVE_LEAVELOBBY(const ByteArray & message, size_t & offset)
{
	return;
}


DGMTProtocol::JoinLobby DGMTProtocol::RECEIVE_JOINLOBBY(const ByteArray & message, size_t & offset)
{
	
	uint32_t lobbyid = m_parser.parse_INTEGER32(message,offset);

	string password = m_parser.parse_STRING8(message,offset);
	uint16_t portnumber = m_parser.parse_INTEGER16(message,offset);

	DSAPublicKey key(m_parser.parse_DSAPublicKey(message,offset));

	return DGMTProtocol::JoinLobby(lobbyid,password,portnumber,key);

}



DGMTProtocol::JoinedLobby DGMTProtocol::RECEIVE_JOINEDLOBBY(const ByteArray & message, size_t & offset)
{
	
	uint32_t lobbyid = m_parser.parse_INTEGER32(message,offset);
	JoinedLobby::answer answer = (JoinedLobby::answer) m_parser.parse_BYTE(message,offset);

	if(answer != DGMTProtocol::JoinedLobby::JOINEDLOBBY_SUCCESS) return DGMTProtocol::JoinedLobby(lobbyid,answer);
		

		string lobbyname = m_parser.parse_USTRING8(message,offset);
		uint8_t maxplayers = m_parser.parse_INTEGER8(message,offset);
		uint8_t creatorpeerid = m_parser.parse_INTEGER8(message,offset);
		uint8_t peerid =  m_parser.parse_INTEGER8(message,offset);
		uint64_t sessionid =  m_parser.parse_INTEGER64(message,offset);
		uint8_t clientlobbylist_size = m_parser.parse_INTEGER8(message,offset);

		vector<ClientLobbyInfo> clientlobbylist;
		clientlobbylist.reserve(clientlobbylist_size);

		for(int i = 1; i<=clientlobbylist_size ;i++)
		{
			uint8_t peerid = m_parser.parse_INTEGER8(message,offset);
			string displayname = m_parser.parse_USTRING8(message,offset);

			uint32_t ipaddress = m_parser.parse_INTEGER32(message,offset);
			uint16_t portnumber = m_parser.parse_INTEGER16(message,offset);
			DSAPublicKey key(m_parser.parse_DSAPublicKey(message,offset));

			clientlobbylist.push_back(ClientLobbyInfo(peerid,displayname,ipaddress,portnumber,key));
		}

		return JoinedLobby(lobbyid,answer,lobbyname,maxplayers,creatorpeerid,peerid,sessionid,move(clientlobbylist));

}


DGMTProtocol::CreateLobby DGMTProtocol::RECEIVE_CREATELOBBY(const ByteArray & message, size_t & offset,  CryptoPP::RSAES_OAEP_SHA_Decryptor Decryptor )
{
	

	string lobbyname = m_parser.parse_USTRING8(message,offset);
	uint8_t maxplayers = m_parser.parse_INTEGER8(message,offset);
	bool haspassword = m_parser.parse_BOOLEAN(message,offset);
	string password="";
	if(haspassword){

		string encryptedpassword = m_parser.parse_STRING16(message,offset);

		try{
			CryptoPP::StringSource( encryptedpassword, true,
				new CryptoPP::PK_DecryptorFilter( m_rng, Decryptor,
				new CryptoPP::StringSink( password )
				) // PK_DecryptorFilter
			); // StringSource
		}catch ( CryptoPP::Exception e)
		{
			cout << "An exception occurred. Exception type: " << e.GetErrorType() << endl;
			throw Err();
		}
	}
	uint16_t portnumber = m_parser.parse_INTEGER16(message,offset);
	DSAPublicKey publickey(m_parser.parse_DSAPublicKey(message,offset));

	return CreateLobby(lobbyname,maxplayers,haspassword,password,portnumber,publickey);

}

DGMTProtocol::LobbyCreation DGMTProtocol::RECEIVE_LOBBYCREATION(const ByteArray & message, size_t & offset)
{

	DGMTProtocol::LobbyCreation::answer answer = (DGMTProtocol::LobbyCreation::answer) m_parser.parse_BYTE(message,offset);
	if(answer == LobbyCreation::LOBBYCREATION_SUCCESS)
	{
		uint32_t lobbyid = m_parser.parse_INTEGER32(message,offset);
		uint8_t peerid =m_parser.parse_INTEGER8(message,offset);
		uint64_t sessionid = m_parser.parse_INTEGER64(message,offset);
		return DGMTProtocol::LobbyCreation(answer,lobbyid,peerid,sessionid);
	}else{	
		return DGMTProtocol::LobbyCreation(answer);
	}

}

DGMTProtocol::LoginReply DGMTProtocol::RECEIVE_LOGINREPLY(const ByteArray & message, size_t & offset)
{
	
	DGMTProtocol::LoginReply::answer answer = (DGMTProtocol::LoginReply::answer) m_parser.parse_BYTE(message,offset);

	if(answer!=DGMTProtocol::LoginReply::LOGINREPLY_SUCCESS && answer !=DGMTProtocol::LoginReply::LOGINREPLY_SUCCESSDISCONNECTEDELSEWHERE) return DGMTProtocol::LoginReply(answer,"");

	string displayname =  m_parser.parse_USTRING8(message,offset);

	return DGMTProtocol::LoginReply(answer,displayname);

}

DGMTProtocol::LoginInfo DGMTProtocol::RECEIVE_LOGIN(const ByteArray & message, size_t & offset, CryptoPP::RSAES_OAEP_SHA_Decryptor Decryptor )
{
	

	string username = m_parser.parse_USTRING8(message,offset);
	string encryptedpassword = m_parser.parse_USTRING16(message,offset);
	string password;
	try{
		CryptoPP::StringSource( encryptedpassword, true,
			new CryptoPP::PK_DecryptorFilter( m_rng, Decryptor,
			new CryptoPP::StringSink( password )
			) // PK_DecryptorFilter
		); // StringSource
	}catch (int e){
		cout << "An exception occurred. Exception Nr. " << e << endl;
		throw Err();
	}


	return DGMTProtocol::LoginInfo(username,password);

}

DGMTProtocol::UserCreation DGMTProtocol::RECEIVE_USERCREATION(const ByteArray & message, size_t & offset)
{
	
	return (DGMTProtocol::UserCreation) m_parser.parse_BYTE(message,offset);
}

DGMTProtocol::CreateUserInfo DGMTProtocol::RECEIVE_CREATEUSER(const ByteArray & message, size_t & offset, CryptoPP::RSAES_OAEP_SHA_Decryptor Decryptor){

	
	string username = m_parser.parse_USTRING8(message,offset);
	string displayname = m_parser.parse_USTRING8(message,offset);
	string email = m_parser.parse_STRING16(message,offset);
	string encryptedpassword = m_parser.parse_STRING16(message,offset);


	string password;
	try{

		CryptoPP::StringSource( encryptedpassword, true,
			new CryptoPP::PK_DecryptorFilter( m_rng, Decryptor,
			new CryptoPP::StringSink( password )
			) // PK_DecryptorFilter
		); // StringSource
	}catch ( CryptoPP::Exception e)
	{
		cout << "An exception occurred. Exception type: " << e.GetErrorType() << endl;
		throw Err();
	}


		return DGMTProtocol::CreateUserInfo(username,displayname,email,password);

}

uint32_t DGMTProtocol::RECEIVE_HELLOFROMCLIENT(const ByteArray & message, size_t & offset)
{
	
	return m_parser.parse_INTEGER32(message,offset);

}


DGMTProtocol::ConnectionReply DGMTProtocol::RECEIVE_HELLOFROMSERVER(const ByteArray & message, size_t & offset)
{
	
	ServerHello answer = (ServerHello) m_parser.parse_BYTE(message,offset);
	RSAPublicKey key(m_parser.parse_RSAPublicKey(message,offset));

	string clientmessage = m_parser.parse_USTRING16(message,offset);

	if(answer == SHELLO_CONNECTED)
	{
		return ConnectionReply(true, key, clientmessage);
	}else
	{
		if(answer == SHELLO_REFUSEDWRONGVERSION)
		{
			return ConnectionReply(false, key, "Server refused connection. Wrong client version:"+clientmessage);
		}else{
			return ConnectionReply(false, key, clientmessage);
		}
	}

}






byte_t DGMTProtocol::GetMessageType(const ByteArray &message, size_t & offset){
	return m_parser.parse_BYTE(message,offset);
}
