#ifndef MT_MINDTRIS_H
#define MT_MINDTRIS_H

#define MINDTRISCLIENT_VERSION "1.0"

class PeerInfo;
class Peer;
class CTCPClient;
class CTCPServer;
class DGMTProtocol;
class DGMTP2PProtocol;

class MindTrisClient : private NonCopyable
{
public:
	enum ClientState{
		STATE_CONNECTING,
		STATE_CONNECTED,
		STATE_SERVERREFUSED,
		STATE_REGISTERINGUSER,
		STATE_REGISTEREDUSER,
		STATE_REGISTRATIONERROR,
		STATE_LOGGINGIN,
		STATE_REFUSEDLOGIN,
		STATE_LOGGEDIN
	};

private:



	unique_ptr<Tetris> m_tetris;

	CryptoPP::AutoSeededRandomPool m_randPool;

	DGMTP2PProtocol m_P2PProtocol;
	DGMTProtocol m_Protocol;
	unique_ptr<CTCPClient> m_Socket;
	unique_ptr<CTCPServer> m_P2PSocket;
	vector<unique_ptr<Peer>>  m_peers;
	vector<unique_ptr<PeerInfo>> m_peerinfovector;
	MessageStreamer m_MessageStreamer;
	queue<Message> m_Messages;
	unique_ptr<Packet> m_IncompletePacket;
	DSAPublicKey m_PublicKey;
	RSAPublicKey m_serverPublicKey;
	unique_ptr<CryptoPP::RSAES_OAEP_SHA_Encryptor> m_Encryptor;
	unique_ptr<CryptoPP::DSA::Signer> m_Signer;
	string m_displayname;
	bool m_DeleteMe;
	uint16_t m_listenport;
	uint32_t m_lobbyid;
	uint8_t m_peerid;
	uint64_t m_sessionid;
	bool m_inalobby;
	bool m_gamestarted;
	string m_ErrorString;
	bool m_Error;
	ClientState m_state;

	vector<uint8_t> new_pieces;
public:

	void UpdateLeaveLobby();

	Tetris & GetGame(){ return *m_tetris;}	

	void SetGameStarted(bool b) { m_gamestarted = b;}
	uint8_t GetPeerID(){ return m_peerid;}
	uint32_t GetLobbyID(){ return m_lobbyid;}

	vector<unique_ptr<PeerInfo>> & GetPeerInfoVector() { return m_peerinfovector;}

	CryptoPP::RandomPool & GetRandomPool(){ return m_randPool;}
	bool ServerStatusUpdate(fd_set * fd);

	void StopP2PSocket();
	void StartP2PSocket();

	vector<unique_ptr<Peer>> & GetPeers() { return m_peers;}
	const RSAPublicKey & GetServerPublicKey(){return m_serverPublicKey;}
	unique_ptr<CryptoPP::RSAES_OAEP_SHA_Encryptor> & GetEncryptor() { return m_Encryptor;}
	unique_ptr<CryptoPP::DSA::Signer> & GetSigner() { return m_Signer;}

    void PrintMalformedMessage();
	string GetDisplayName(){ return m_displayname;}
	void SetDisplayName( string s){ m_displayname = s;}

	void SetClientState(ClientState s ){ m_state = s;}
	ClientState GetClientState(){ return m_state;}

	void RegisterUser(string username, string displayname, string email, string password);
	void RetrieveLobbyList();
	void StartGame();
	void CreateLobby(string lobbyname, uint8_t maxplayers,bool haspassword,string password);
	void JoinLobby(uint32_t lobbyid, string password);
	void LeaveLobby();
	void SetInALobby(bool b){ m_inalobby = b;}
	bool InALobby(){ return m_inalobby;}
	void Login(string username, string password);
	void SendChatCommand(string message);
	string GetVersionS(){ return MINDTRISCLIENT_VERSION;}
	MindTrisClient(string address, uint16_t port, uint16_t clientport);
	bool Update(long usecBlock);

};

void CONSOLE_Print(string s);

#endif //MT_MINDTRIS_H