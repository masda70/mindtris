#ifndef DGMTPROTOCOL_H
#define DGMTPROTOCOL_H


#define DGMT_PROTOCOLIDENTIFIER "DGMT"

#define DGMT_PROTOCOL_VERSION 0x01020003

class ByteArray;
class RSAPublicKey;
class DSAPublicKey;
class MessageParser;

class DGMTProtocol :  NonCopyable
{
private:
	MessageParser m_parser;
	CryptoPP::AutoSeededRandomPool m_rng;
public:
	const uint32_t static PROTOCOLVERSION = DGMT_PROTOCOL_VERSION;
	const ByteArray & GetProtocolIdentifier() const;
	const bool IsBigEndian() const ;
	DGMTProtocol();
	class Err{public: Err(){}};

	enum Protocol {
		TYPE_KEEPALIVE				= 0xFF,
		TYPE_HELLOFROMCLIENT		= 0x00,	
		TYPE_CREATEUSER				= 0x01,
		TYPE_LOGIN					= 0x02,
		TYPE_CREATELOBBY			= 0x03,
		TYPE_GETLOBBYLIST			= 0x04,
		TYPE_JOINLOBBY				= 0x05,
		TYPE_LEAVELOBBY				= 0x06,
		TYPE_KICKUSERFROMLOBBY		= 0x07,

		TYPE_STARTGAME				= 0x10,
		TYPE_LOADEDGAME				= 0x11,
		TYPE_GIVENEWPIECES			= 0x13,
		TYPE_GAMEEND				= 0x14,

		TYPE_HELLOFROMSERVER		= 0x80,
		TYPE_USERCREATION			= 0x81,
		TYPE_LOGINREPLY				= 0x82,
		TYPE_LOBBYCREATION			= 0x83,
		TYPE_LOBBYLIST				= 0x84,
		TYPE_JOINEDLOBBY			= 0x85,

		TYPE_UPDATECLIENTSTATUS		= 0x88,
		TYPE_GAMESTARTING			= 0x90,
		TYPE_LOADGAME				= 0x91,
		TYPE_BEGINGAME				= 0x92,
		TYPE_NEWPIECES				= 0x93,
	};

	enum GameStarting {
		GAMESTARTING_STARTING		= 0x00,
	};

	enum LoadedGame {
		LOADEDGAME_READY			= 0x00,
		LOADEDGAME_CANTCONNECTTOPEERS	= 0x01,
	};


	enum ServerHello {

		SHELLO_CONNECTED			= 0x00,
		SHELLO_REFUSEDWRONGVERSION	= 0x01,
		SHELLO_REFUSEDUNKNOWNERROR	= 0x02
	};
	enum UserCreation {

		USERCREATION_SUCCESS					= 0x00,
		USERCREATION_USERNAMEALREADYEXISTS		= 0x01,
		USERCREATION_INVALIDUSERNAME			= 0x02,
		USERCREATION_INVALIDPASSWORD			= 0x03,
		USERCREATION_INVALIDEMAIL				= 0x04,
		USERCREATION_INVALIDPACKET				= 0xFF
	};


	class ClientLobbyInfo
	{
	private:
		uint8_t m_peerid;
		string m_displayname;
		uint32_t m_ipaddress;
		uint16_t m_portnumber;
		DSAPublicKey m_publickey;
	public:
		uint8_t GetPeerID() const{ return m_peerid;}
		string GetDisplayName() const { return m_displayname;}
		uint32_t GetIPAddress() const{ return m_ipaddress;}
		uint16_t GetPortNumber() const{ return m_portnumber;}
		DSAPublicKey GetPublicKey() const{ return m_publickey;}
		ClientLobbyInfo(
			uint8_t peerid
			) : m_publickey()
		{
			m_peerid = peerid;
			m_displayname = "";
			m_ipaddress = 0;
			m_portnumber = 0;
		}
		ClientLobbyInfo(	
			uint8_t peerid,
			string displayname,
			uint32_t ipaddress,
			uint16_t portnumber,
			DSAPublicKey publickey): m_publickey()
		{
			m_peerid = peerid;
			m_displayname = displayname; 
			m_ipaddress = ipaddress;
			m_portnumber = portnumber;
			m_publickey = move(publickey);
		}
	};
	class JoinedLobby
	{
	public:
	
		enum answer {

			JOINEDLOBBY_SUCCESS				= 0x00,
			JOINEDLOBBY_WRONGPASSWORD 		= 0x01,
			JOINEDLOBBY_LOBBYFULL			= 0x02,
			JOINEDLOBBY_UNKNOWNERROR		= 0x03
		};
	private:
		uint32_t m_lobbyid;
		string m_lobbyname;
		uint8_t m_maxplayers;
		uint8_t m_creatorpeerid;
		answer m_answer;
		uint8_t m_peerid;
		uint64_t m_sessionid;
		vector<ClientLobbyInfo> m_clientlobbylist;
	public:
		uint8_t GetCreatorPeerID() const {return m_creatorpeerid;}
		uint8_t GetMaxPlayers() const {return m_maxplayers;}
		string GetLobbyName() const {return m_lobbyname;}
		uint32_t GetLobbyID() const{return m_lobbyid;}
		answer GetAnswer()const{return m_answer;}
		uint8_t GetPeerID() const{ return m_peerid;}
		uint64_t GetSessionID() const{ return m_sessionid;}
		const vector<ClientLobbyInfo> & GetClientLobbyList() const{ return m_clientlobbylist;}
		JoinedLobby(
				uint32_t lobbyid,
				answer answer
				)
		{
			m_lobbyid = lobbyid;
			m_answer = answer;
			m_peerid = 0;
			m_sessionid = 0;
		}
		JoinedLobby(
				uint32_t lobbyid,
				answer answer,
				string lobbyname,
				uint8_t maxplayers,
				uint8_t creatorpeerid,
				uint8_t peerid,
				uint64_t sessionid,
				vector<ClientLobbyInfo> clientlobbylist
				)
				:
			m_lobbyid(lobbyid),
			m_answer(answer),
			m_lobbyname(lobbyname),
			m_maxplayers(maxplayers),
			m_creatorpeerid(creatorpeerid),
			m_peerid(peerid),
			m_sessionid(sessionid),
			m_clientlobbylist(move(clientlobbylist))
			{
		}

	};

	class JoinLobby
	{
	private:
		uint32_t m_lobbyid;
		string m_password;
		uint16_t m_portnumber;
		DSAPublicKey m_publickey;
	public:
		uint32_t GetLobbyID() const{return m_lobbyid;}
		string GetPassword() const{ return m_password;}
		uint16_t GetPortNumber() const{ return m_portnumber;}
		DSAPublicKey GetPublicKey() const{return m_publickey;}
		JoinLobby(
				uint32_t lobbyid,
				string password,
				uint16_t portnumber,
				DSAPublicKey publickey
				) : m_publickey()
		{
			m_lobbyid = lobbyid;
			m_password = password;
			m_portnumber = portnumber;
			m_publickey = move(publickey);
		}
	};



	class LobbyCreation
	{
	public:
		enum answer {
			LOBBYCREATION_SUCCESS			= 0x00,
			LOBBYCREATION_INVALIDPASSWORD	= 0x01,
			LOBBYCREATION_NOTENOUGHRIGHTS	= 0x02,
			LOBBYCREATION_INVALIDNUMBEROFPLAYERS	= 0x03,
			LOBBYCREATION_UNKNOWNERROR		= 0xFF
		};
	private:
		answer m_answer;
		uint32_t m_lobbyid;
		uint8_t m_peerid;
		uint64_t m_sessionid;
	public:

		answer GetAnswer() const{return m_answer;}
		uint32_t GetLobbyID() const{return m_lobbyid;}
		uint8_t GetPeerID() const {return m_peerid;}
		uint64_t GetSessionID() const{return m_sessionid;}
		LobbyCreation(answer answer): m_answer(answer){ m_lobbyid = 0; m_peerid=0; m_sessionid = 0;}
		LobbyCreation(answer answer, uint32_t lobbyid, uint8_t peerid, uint64_t sessionid): m_answer(answer), m_lobbyid(lobbyid), m_peerid(peerid), m_sessionid(sessionid){}
	};

	class LobbyInfo
	{
	private:
		uint32_t m_lobbyid;
		string m_lobbyname;
		uint8_t m_playercount;
		uint8_t m_maxplayers;
		bool m_haspassword;
		string m_creatordisplayname;
	public:
		uint32_t GetLobbyId() const{return m_lobbyid;}
		string GetLobbyName() const{return m_lobbyname;}
		uint8_t GetPlayerCount() const{return m_playercount;}
		uint8_t GetMaxPlayers() const {return m_maxplayers;}
		bool GetHasPassword() const{return m_haspassword;}
		string GetCreatorDisplayName() const {return m_creatordisplayname;}
		LobbyInfo(uint32_t lobbyid, string lobbyname, uint8_t playercount, uint8_t maxplayers, bool haspassword, string creatordisplayname): m_lobbyid(lobbyid), m_lobbyname(lobbyname), m_playercount(playercount), m_maxplayers(maxplayers), m_haspassword(haspassword), m_creatordisplayname(creatordisplayname) {}
	};


	class CreateLobby
	{
	private:
		string m_lobbyname;
		uint8_t m_maxplayers;
		bool m_haspassword;
		string m_password;
		uint16_t m_portnumber;
		DSAPublicKey m_publickey;
	public:
		string GetLobbyName() const {return m_lobbyname;}
		uint8_t GetMaxPlayers() const{return m_maxplayers;}
		bool GetHasPassword() const {return m_haspassword;}
		string GetPassword() const {return m_password;}
		uint16_t GetPortNumber() const{ return m_portnumber;}
		DSAPublicKey GetPublicKey() const{return m_publickey;}
		CreateLobby(string lobbyname, uint8_t maxplayers, bool haspassword, string password, uint16_t portnumber, DSAPublicKey key): m_lobbyname(lobbyname), m_maxplayers(maxplayers), m_haspassword(haspassword), m_password(password), m_portnumber(portnumber), m_publickey(key) {}
	};

	class LoginReply
	{
	public:
		enum answer {
			LOGINREPLY_SUCCESS							= 0x00,
			LOGINREPLY_USERNAMEDOESNOTEXIST				= 0x01,
			LOGINREPLY_BADUSERNAMEPASSWORD				= 0x02,
			LOGINREPLY_TOOMANYTRIES						= 0x03,
			LOGINREPLY_SUCCESSDISCONNECTEDELSEWHERE		= 0x04
		};
	private:
		answer m_Answer;
		string m_DisplayName;
	public:

		answer GetAnswer() const{return m_Answer;}
		const string GetDisplayName() const {return m_DisplayName;}
		LoginReply(answer answer, string displayname): m_Answer(answer), m_DisplayName(displayname){}
	};

	class LoginInfo
	{
	private:
		string m_Username;
		string m_Password;
	public:
		string GetUsername() const {return m_Username;}
		string GetPassword() const {return m_Password;}
		LoginInfo(string username, string password): m_Username(username), m_Password(password){}
	};


	class CreateUserInfo
	{
	private:
		string m_Username;
		string m_DisplayName;
		string m_Email;
		string m_Password;
	public:
		string GetUsername() const {return m_Username;}
		string GetDisplayName() const {return m_DisplayName;}
		string GetEmail() const {return m_Email;}
		string GetPassword() const {return m_Password;}
		CreateUserInfo(string username, string displayname, string email, string password):m_Username(username),	m_DisplayName(displayname), m_Email(email), m_Password(password){}
	};


	class UpdateClientStatus
	{
	public:
		enum status {

			STATUSUPDATE_HASJOINEDTHELOBBY	= 0x00,
			STATUSUPDATE_HASLEFTTHELOBBY 		= 0x01,
			STATUSUPDATE_HASBEENKICKED		= 0x02
		};
	private:
		status m_statusupdate;
		uint32_t m_lobbyid;
		ClientLobbyInfo m_clientlobbyinfo;
	public:
		status GetStatusUpdate() const { return m_statusupdate;}
		uint32_t GetLobbyID() const { return m_lobbyid;}
		ClientLobbyInfo GetClientLobbyInfo() const{ return m_clientlobbyinfo;}
		UpdateClientStatus(status statusupdate, uint32_t lobbyid, ClientLobbyInfo clientlobbyinfo): m_statusupdate (statusupdate), m_lobbyid(lobbyid), m_clientlobbyinfo(move(clientlobbyinfo)){};
	};

	class ConnectionReply
	{
	private:
		bool m_Connected;
		RSAPublicKey m_publickey;
		string m_Message;
	public:
		const RSAPublicKey GetPublicKey() const{return m_publickey;}
		string GetMessage() const{return m_Message;}
		bool Connected() const {return m_Connected;}
		ConnectionReply(bool nConnected, RSAPublicKey publickey, string nMessage):	m_Connected(nConnected),m_publickey(move(publickey)), m_Message(nMessage){}
	};


	class GiveNewPieces
	{
	private:
		uint32_t m_offset;
		uint8_t m_number;
	public:
		uint8_t GetNumber() const{return m_number;}
		uint32_t GetOffset() const{return m_offset;}
		GiveNewPieces(uint32_t offset, uint8_t number):	m_offset(offset), m_number(number){}
	};

	class NewPieces
	{
	private:
		uint32_t m_offset;
		vector<uint8_t> m_pieces;
	public:
		uint32_t GetOffset() const{return m_offset;}
		const vector<uint8_t> & GetPieces() const{ return m_pieces;}
		NewPieces(uint32_t offset, vector<uint8_t> pieces): m_offset(offset),m_pieces(pieces){}
	};





	//SEND FUNCTIONS
	ByteArray SEND_KEEPALIVE();

		// GET_LOBBY_LIST - LOBBY_LIST
		ByteArray SEND_GETLOBBYLIST();
		ByteArray SEND_LOBBYLIST(const vector<LobbyInfo>&);

		//UPDATECLIENTSTATUS
		ByteArray SEND_UPDATECLIENTSTATUS(UpdateClientStatus::status answer, uint32_t lobbyid, uint8_t peerid, const string & DisplayName, uint32_t ipaddress, uint16_t portnumber, const DSAPublicKey & publickey);
		ByteArray SEND_UPDATECLIENTSTATUS(UpdateClientStatus::status answer, uint32_t lobbyid, uint8_t peerid);
		//LEAVE_LOBBY 
		ByteArray SEND_LEAVELOBBY();

		//JOIN_LOBBY - JOINED_LOBBY
		ByteArray SEND_JOINLOBBY(uint32_t lobbyid, const string & password, uint16_t portnumber,const DSAPublicKey & public_key);

		ByteArray SEND_JOINEDLOBBY(uint32_t lobbyid, JoinedLobby::answer answer);
		ByteArray SEND_JOINEDLOBBY(uint32_t lobbyid, JoinedLobby::answer answer, const string & lobbyname,  uint8_t maxplayers,  uint8_t creatorid, uint8_t peerid, uint64_t sessionid, const vector<ClientLobbyInfo> & clientinfolist);

		//CREATE_LOBBY - LOBBY_CREATION
		ByteArray SEND_CREATELOBBY(const string & lobbyname, uint8_t maxplayers, bool haspassword, const string & password,  const CryptoPP::PK_Encryptor & encryptor,uint16_t portnumber,const DSAPublicKey & publickey );
		ByteArray SEND_LOBBYCREATION(LobbyCreation::answer answer, uint32_t lobbyid, uint8_t peerid, uint64_t sessionid);
		ByteArray SEND_LOBBYCREATION(LobbyCreation::answer answer);

		//CREATE_USER - USER_CREATION
		ByteArray SEND_CREATEUSER(const string & username, const string & display_name, const string & email, const string & password,  const CryptoPP::PK_Encryptor & encryptor );
		ByteArray SEND_USERCREATION(UserCreation answer);
		
		//HELLO_FROM_CLIENT - HELLO_FROM_SERVER
		ByteArray SEND_HELLOFROMCLIENT();
		ByteArray SEND_HELLOFROMSERVER(ServerHello answer, const RSAPublicKey & publickey, const string & Message);

		//LOGIN - LOGINREPLY
		ByteArray SEND_LOGIN(const string & username, const string & password, const CryptoPP::PK_Encryptor & encryptor);
		ByteArray SEND_LOGINREPLY(LoginReply::answer reply, const string & display_name);


		//START_GAME, GAME_STARTING
		ByteArray SEND_STARTGAME();
		ByteArray SEND_GAMESTARTING(GameStarting answer);

		//START_GAME, GAME_STARTING
		ByteArray SEND_LOADGAME(const vector<byte_t> & pieces);
		ByteArray SEND_LOADEDGAME(LoadedGame answer);


		//BEGINGAME
		ByteArray SEND_BEGINGAME();


		// NEWPIECES, GIVENEWPIECES
		ByteArray SEND_NEWPIECES(uint32_t offset, const vector<byte_t> & pieces);
		ByteArray SEND_GIVENEWPIECES(uint32_t offset, uint8_t number);


	//RECEIVE FUNCTIONS


		void RECEIVE_STARTGAME(const ByteArray & message, size_t & offset);
		GameStarting RECEIVE_GAMESTARTING(const ByteArray & message, size_t & offset);

		vector<byte_t> RECEIVE_LOADGAME(const ByteArray & message, size_t & offset);
		LoadedGame RECEIVE_LOADEDGAME(const ByteArray & message, size_t & offset);

		void RECEIVE_BEGINGAME(const ByteArray & message, size_t & offset);

		NewPieces RECEIVE_NEWPIECES(const ByteArray & message, size_t & offset);
		GiveNewPieces RECEIVE_GIVENEWPIECES(const ByteArray & message, size_t & offset);



		void RECEIVE_GETLOBBYLIST(const ByteArray & message, size_t & offset);
		vector<LobbyInfo> RECEIVE_LOBBYLIST(const ByteArray & message, size_t & offset);

		//UPDATECLIENTSTATUS
		UpdateClientStatus RECEIVE_UPDATECLIENTSTATUS(const ByteArray & message, size_t & offset);

		//LEAVE_LOBBY, might be needed in the future 

		void RECEIVE_LEAVELOBBY(const ByteArray & message, size_t & offset); 

		//JOIN_LOBBY - JOINED_LOBBY
		JoinLobby RECEIVE_JOINLOBBY(const ByteArray & message, size_t & offset);
		JoinedLobby RECEIVE_JOINEDLOBBY(const ByteArray & message, size_t & offset);

		//CREATE_LOBBY - LOBBY_CREATION
		CreateLobby RECEIVE_CREATELOBBY(const ByteArray & message, size_t & offset,  CryptoPP::RSAES_OAEP_SHA_Decryptor Decryptor );
		LobbyCreation RECEIVE_LOBBYCREATION(const ByteArray & message, size_t & offset);

		//LOGIN - LOGINREPLY
		LoginInfo RECEIVE_LOGIN(const ByteArray & message, size_t & offset, CryptoPP::RSAES_OAEP_SHA_Decryptor Decryptor );
		LoginReply RECEIVE_LOGINREPLY(const ByteArray & message, size_t & offset);

		//CREATE_USER - USER_CREATION
		CreateUserInfo RECEIVE_CREATEUSER(const ByteArray & message, size_t & offset, CryptoPP::RSAES_OAEP_SHA_Decryptor Decryptor );
		UserCreation RECEIVE_USERCREATION(const ByteArray & message, size_t & offset);

		//HELLO_FROM_CLIENT - HELLO_FROM_SERVER
		uint32_t RECEIVE_HELLOFROMCLIENT(const ByteArray & message, size_t & offset);
		ConnectionReply RECEIVE_HELLOFROMSERVER(const ByteArray & message, size_t & offset);

		byte_t GetMessageType(const ByteArray &message, size_t & offset);




};



#endif











