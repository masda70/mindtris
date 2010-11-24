#ifndef DGMTPROTOCOL_H
#define DGMTPROTOCOL_H


#define DGMT_HEADER_CONSTANT1 'D'
#define DGMT_HEADER_CONSTANT2 'G'
#define DGMT_HEADER_CONSTANT3 'M'
#define DGMT_HEADER_CONSTANT4 'T'

#define DGMT_PROTOCOL_VERSION 0x00000001

class DGMTUpdateClientStatus;

class DGMTLobbyInfo;
class DGMTJoinLobby;
class DGMTJoinedLobby;

class DGMTClientLobbyInfo;

class DGMTLoginInfo;
class DGMTLoginReply;
class DGMTClientUpdate;

class DGMTConnectionReply;
class DGMTPacket;
class DGMTCreateUserInfo;
class DGMTCreateLobby;
class DGMTLobbyCreation;


class DGMTProtocol
{

public:
	const int static HEADERLENGTH = 6;
	const uint32_t static DGMT_PROTOCOLVERSION = DGMT_PROTOCOL_VERSION;

	enum Protocol {
		DGMT_KEEPALIVE				= 0xFF,
		DGMT_HELLOFROMCLIENT		= 0x00,	
		DGMT_CREATEUSER				= 0x01,
		DGMT_LOGIN					= 0x02,
		DGMT_CREATELOBBY			= 0x03,
		DGMT_GETLOBBYLIST			= 0x04,
		DGMT_JOINLOBBY				= 0x05,
		DGMT_LEAVELOBBY				= 0x06,
		DGMT_KICKUSERFROMLOBBY		= 0x07,

		DGMT_STARGAME				= 0x10,
		DGMT_LOADEDGAME				= 0x11,
		DGMT_GIVENEWPIECES			= 0x13,
		DGMT_GAMEEND				= 0x14,

		DGMT_HELLOFROMSERVER		= 0x80,
		DGMT_USERCREATION			= 0x81,
		DGMT_LOGINREPLY				= 0x82,
		DGMT_LOBBYCREATION			= 0x83,
		DGMT_LOBBYLIST				= 0x84,
		DGMT_JOINEDLOBBY			= 0x85,

		DGMT_UPDATECLIENTSTATUS		= 0x88,
		DGMT_GAMESTARTING			= 0x90,
		DGMT_LOADGAME				= 0x91,
		DGMT_BEGINGAME				= 0x92,
		DGMT_NEWPIECES				= 0x93,
	};

	enum LoginReply {

		LOGINREPLY_SUCCESS							= 0x00,
		LOGINREPLY_USERNAMEDOESNOTEXIST				= 0x01,
		LOGINREPLY_BADUSERNAMEPASSWORD				= 0x02,
		LOGINREPLY_TOOMANYTRIES						= 0x03,
		LOGINREPLY_SUCCESSDISCONNECTEDELSEWHERE		= 0x04
	};

	enum UserCreation {

		USERCREATION_SUCCESS					= 0x00,
		USERCREATION_USERNAMEALREADYEXISTS		= 0x01,
		USERCREATION_INVALIDUSERNAME			= 0x02,
		USERCREATION_INVALIDPASSWORD			= 0x03,
		USERCREATION_INVALIDEMAIL				= 0x04,
		USERCREATION_INVALIDPACKET				= 0xFF
	};

	enum ServerHello {

		SHELLO_CONNECTED			= 0x00,
		SHELLO_REFUSEDWRONGVERSION	= 0x01,
		SHELLO_REFUSEDUNKNOWNERROR	= 0x02
	};

	enum LobbyCreation {

		LOBBYCREATION_SUCCESS			= 0x00,
		LOBBYCREATION_INVALIDPASSWORD	= 0x01,
		LOBBYCREATION_NOTENOUGHRIGHTS	= 0x02
	};

	enum JoinedLobby {

		JOINEDLOBBY_SUCCESS				= 0x00,
		JOINEDLOBBY_WRONGPASSWORD 		= 0x01,
		JOINEDLOBBY_LOBBYFULL			= 0x02,
		JOINEDLOBBY_UNKNOWNERROR		= 0x03
	};

	enum StatusUpdate {

		STATUSUPDATE_HASJOINEDTHELOBBY	= 0x00,
		STATUSUPDATE_HASLEFTTHELOBBY 		= 0x01,
		STATUSUPDATE_HASBEENKICKED		= 0x02
	};

	//SEND FUNCTIONS
	BYTEARRAY SEND_DGMT_KEEPALIVE();

		// GET_LOBBY_LIST - LOBBY_LIST
		BYTEARRAY SEND_DGMT_GETLOBBYLIST();
		BYTEARRAY SEND_DGMT_LOBBYLIST(vector<DGMTLobbyInfo>*);

		//UPDATECLIENTSTATUS
		BYTEARRAY SEND_DGMT_UPDATECLIENTSTATUS(StatusUpdate answer,uint8_t peerid, string DisplayName, uint32_t ipaddress, uint16_t portnumber, RSAPublicKey * publickey);
		BYTEARRAY SEND_DGMT_UPDATECLIENTSTATUS(StatusUpdate answer, uint8_t peerid);
		//LEAVE_LOBBY 
		BYTEARRAY SEND_DGMT_LEAVELOBBY();

		//JOIN_LOBBY - JOINED_LOBBY
		BYTEARRAY SEND_DGMT_JOINLOBBY(uint32_t lobbyid, string password, uint16_t portnumber,RSAPublicKey * public_key);

		BYTEARRAY SEND_DGMT_JOINEDLOBBY(uint32_t lobbyid, JoinedLobby answer);
		BYTEARRAY SEND_DGMT_JOINEDLOBBY(uint32_t lobbyid, JoinedLobby answer, uint8_t peerid, uint64_t sessionid, vector<DGMTClientLobbyInfo> * clientinfolist);

		//CREATE_LOBBY - LOBBY_CREATION
		BYTEARRAY SEND_DGMT_CREATELOBBY(string lobbyname, uint8_t maxplayers, bool haspassword, string password,  CryptoPP::PK_Encryptor * encryptor );
		BYTEARRAY SEND_DGMT_LOBBYCREATION(LobbyCreation answer, uint32_t lobbyid, uint64_t sessionid);


		//CREATE_USER - USER_CREATION
		BYTEARRAY SEND_DGMT_CREATEUSER(string username, string display_name, string email, string password,  CryptoPP::PK_Encryptor * encryptor );
		BYTEARRAY SEND_DGMT_USERCREATION(UserCreation answer);
		
		//HELLO_FROM_CLIENT - HELLO_FROM_SERVER
		BYTEARRAY SEND_DGMT_HELLOFROMCLIENT();
		BYTEARRAY SEND_DGMT_HELLOFROMSERVER(ServerHello answer, RSAPublicKey * publickey, string Message);

		//LOGIN - LOGINREPLY
		BYTEARRAY SEND_DGMT_LOGIN(string username, string password, CryptoPP::PK_Encryptor * encryptor);
		BYTEARRAY SEND_DGMT_LOGINREPLY(LoginReply reply, string display_name);


	//RECEIVE FUNCTIONS

		void RECEIVE_DGMT_GETLOBBYLIST(BYTEARRAY data);
		vector<DGMTLobbyInfo> * RECEIVE_DGMT_LOBBYLIST(BYTEARRAY data);

		//UPDATECLIENTSTATUS
		DGMTUpdateClientStatus * RECEIVE_DGMT_UPDATECLIENTSTATUS(BYTEARRAY data);

		//LEAVE_LOBBY, might be needed in the future 

		void RECEIVE_DGMT_LEAVELOBBY(BYTEARRAY data); 

		//JOIN_LOBBY - JOINED_LOBBY
		DGMTJoinLobby * RECEIVE_DGMT_JOINLOBBY(BYTEARRAY data);
		DGMTJoinedLobby * RECEIVE_DGMT_JOINEDLOBBY(BYTEARRAY data);

		//CREATE_LOBBY - LOBBY_CREATION
		DGMTCreateLobby * RECEIVE_DGMT_CREATELOBBY(BYTEARRAY data,  CryptoPP::PK_Decryptor * Decryptor );
		DGMTLobbyCreation * RECEIVE_DGMT_LOBBYCREATION(BYTEARRAY data);

		//LOGIN - LOGINREPLY
		DGMTLoginInfo * RECEIVE_DGMT_LOGIN( BYTEARRAY data, CryptoPP::PK_Decryptor * Decryptor );
		DGMTLoginReply * RECEIVE_DGMT_LOGINREPLY(BYTEARRAY data);

		//CREATE_USER - USER_CREATION
		DGMTCreateUserInfo * RECEIVE_DGMT_CREATEUSER( BYTEARRAY data, CryptoPP::PK_Decryptor * Decryptor );
		UserCreation RECEIVE_DGMT_USERCREATION(BYTEARRAY data);

		//HELLO_FROM_CLIENT - HELLO_FROM_SERVER
		uint32_t RECEIVE_DGMT_HELLOFROMCLIENT( BYTEARRAY data);
		DGMTConnectionReply * RECEIVE_DGMT_HELLOFROMSERVER( BYTEARRAY data);

	//Other functions
	bool ExtractPacket( BYTEARRAY Bytes, DGMTPacket ** packet);

	DGMTProtocol(bool bigEndian);
private:
	bool m_isBigEndian;
	CryptoPP::AutoSeededRandomPool m_rng;
	bool AssignLength( BYTEARRAY &content );
	bool ValidateLength( BYTEARRAY &content );
	bool AppendHeader( BYTEARRAY &content );

};



class DGMTJoinedLobby
{
private:
	uint32_t m_lobbyid;
	DGMTProtocol::JoinedLobby m_answer;
	uint8_t m_peerid;
	uint64_t m_sessionid;
	vector<DGMTClientLobbyInfo> * m_clientlobbylist;
public:
	uint32_t GetLobbyID(){return m_lobbyid;}
	DGMTProtocol::JoinedLobby GetAnswer(){return m_answer;}
	uint8_t GetPeerID(){ return m_peerid;}
	uint64_t GetSessionID(){ return m_sessionid;}
	vector<DGMTClientLobbyInfo> * GetClientLobbyList(){ return m_clientlobbylist;}
	DGMTJoinedLobby(
			uint32_t lobbyid,
			DGMTProtocol::JoinedLobby answer
			)
	{
		m_lobbyid = lobbyid;
		m_answer = answer;
		m_peerid = 0;
		m_sessionid = 0;
		m_clientlobbylist = NULL;
	}
	DGMTJoinedLobby(
			uint32_t lobbyid,
			DGMTProtocol::JoinedLobby answer,
			uint8_t peerid,
			uint64_t sessionid,
			vector<DGMTClientLobbyInfo> * clientlobbylist
			)
	{
		m_lobbyid = lobbyid;
		m_answer = answer;
		m_peerid = peerid;
		m_sessionid = sessionid;
		m_clientlobbylist = clientlobbylist;
	}
	~DGMTJoinedLobby(){
		if(m_clientlobbylist != NULL) delete m_clientlobbylist;
	}

};

class DGMTJoinLobby
{
private:
	uint32_t m_lobbyid;
	string m_password;
	uint16_t m_portnumber;
	RSAPublicKey * m_publickey;
public:
	uint32_t GetLobbyID(){return m_lobbyid;}
	string GetPassword(){ return m_password;}
	uint16_t GetPortNumber(){ return m_portnumber;}
	RSAPublicKey * GetPublicKey(){return m_publickey;}
	DGMTJoinLobby(
			uint32_t lobbyid,
			string password,
			uint16_t portnumber,
			RSAPublicKey * publickey
			)
	{
		m_lobbyid = lobbyid;
		m_password = password;
		m_portnumber = portnumber;
		m_publickey = publickey;
	}
};

class DGMTClientLobbyInfo
{
private:
	uint8_t m_peerid;
	string m_displayname;
	uint32_t m_ipaddress;
	uint16_t m_portnumber;
	RSAPublicKey * m_publickey;
public:
	uint8_t GetPeerID(){ return m_peerid;}
	string GetDisplayName(){ return m_displayname;}
	uint32_t GetIPAddress(){ return m_ipaddress;}
	uint16_t GetPortNumber(){ return m_portnumber;}
	RSAPublicKey * GetPublicKey(){ return m_publickey;}
	DGMTClientLobbyInfo(
		uint8_t peerid
		)
	{
		m_peerid = peerid;
		m_displayname = "";
		m_ipaddress = 0;
		m_portnumber = 0;
		m_publickey = NULL;
	}
	DGMTClientLobbyInfo(	
		uint8_t peerid,
		string displayname,
		uint32_t ipaddress,
		uint16_t portnumber,
		RSAPublicKey * publickey)
	{
		m_peerid = peerid;
		m_displayname = displayname; 
		m_ipaddress = ipaddress;
		m_portnumber = portnumber;
		m_publickey = publickey;
	}
};

class DGMTLobbyCreation
{
private:
	DGMTProtocol::LobbyCreation m_answer;
	uint32_t m_lobbyid;
	uint64_t m_sessionid;
public:
	DGMTProtocol::LobbyCreation GetAnswer(){return m_answer;}
	uint32_t GetLobbyID(){return m_lobbyid;}
	uint64_t GetSessionID(){return m_sessionid;}
	DGMTLobbyCreation(DGMTProtocol::LobbyCreation answer, uint32_t lobbyid, uint64_t sessionid){m_answer = answer; m_lobbyid = lobbyid; m_sessionid = sessionid; }
};

class DGMTLobbyInfo
{
private:
	uint32_t m_lobbyid;
	string m_lobbyname;
	uint8_t m_playercount;
	uint8_t m_maxplayers;
	bool m_haspassword;
	string m_creatordisplayname;
public:
	uint32_t GetLobbyId(){return m_lobbyid;}
	string GetLobbyName(){return m_lobbyname;}
	uint8_t GetPlayerCount(){return m_playercount;}
	uint8_t GetMaxPlayers(){return m_maxplayers;}
	bool GetHasPassword(){return m_haspassword;}
	string GetCreatorDisplayName(){return m_creatordisplayname;}
	DGMTLobbyInfo(uint32_t lobbyid, string lobbyname, uint8_t playercount, uint8_t maxplayers, bool haspassword, string creatordisplayname) {m_lobbyid = lobbyid; m_lobbyname = lobbyname; m_playercount = playercount; m_maxplayers = maxplayers; m_haspassword = haspassword; m_creatordisplayname = creatordisplayname;}
};


class DGMTCreateLobby
{
private:
	string m_lobbyname;
	uint8_t m_maxplayers;
	bool m_haspassword;
	string m_password;
public:
	string GetLobbyName(){return m_lobbyname;}
	uint8_t GetMaxPlayers(){return m_maxplayers;}
	bool GetHasPassword(){return m_haspassword;}
	string GetPassword(){return m_password;}
	DGMTCreateLobby(string lobbyname, uint8_t maxplayers, bool haspassword, string password){m_lobbyname = lobbyname; m_maxplayers = maxplayers; m_haspassword = haspassword; m_password = password;}
};

class DGMTLoginReply
{
private:
	DGMTProtocol::LoginReply m_Answer;
	string m_DisplayName;
public:
	DGMTProtocol::LoginReply GetAnswer(){return m_Answer;}
	string GetDisplayName(){return m_DisplayName;}
	DGMTLoginReply(DGMTProtocol::LoginReply answer, string displayname){m_Answer = answer; m_DisplayName = displayname;}
};

class DGMTLoginInfo
{
private:
	string m_Username;
	string m_Password;
public:
	string GetUsername(){return m_Username;}
	string GetPassword(){return m_Password;}
	DGMTLoginInfo(string username, string password){m_Username=username; m_Password=password;}
};


class DGMTCreateUserInfo
{
private:
	string m_Username;
	string m_DisplayName;
	string m_Email;
	string m_Password;
public:
	string GetUsername(){return m_Username;}
	string GetDisplayName(){return m_DisplayName;}
	string GetEmail(){return m_Email;}
	string GetPassword(){return m_Password;}
	DGMTCreateUserInfo(string username, string displayname, string email, string password){m_Username=username;	m_DisplayName=displayname; m_Email=email; m_Password=password;}
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

class DGMTUpdateClientStatus
{
private:
	DGMTProtocol::StatusUpdate m_statusupdate;
	DGMTClientLobbyInfo * m_clientlobbyinfo;
public:
	DGMTProtocol::StatusUpdate GetStatusUpdate(){ return m_statusupdate;}
	DGMTClientLobbyInfo * GetClientLobbyInfo(){ return m_clientlobbyinfo;}
	DGMTUpdateClientStatus(DGMTProtocol::StatusUpdate statusupdate, DGMTClientLobbyInfo * clientlobbyinfo)
	{
		m_statusupdate = statusupdate;
		m_clientlobbyinfo = clientlobbyinfo;
	}
	~DGMTUpdateClientStatus(){
		delete m_clientlobbyinfo;
	}
};

class DGMTConnectionReply
{
private:
	bool m_Connected;
	RSAPublicKey * m_publickey;
	string m_Message;
public:
	RSAPublicKey * GetPublicKey(){return m_publickey;}
	string GetMessage(){return m_Message;}
	bool Connected(){return m_Connected;}
	DGMTConnectionReply(bool nConnected, RSAPublicKey * publickey, string nMessage){	m_Connected = nConnected;	m_publickey = publickey; m_Message = nMessage;}
};




#endif