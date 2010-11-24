#ifndef MT_MINDTRIS_H
#define MT_MINDTRIS_H

#define MINDTRISCLIENT_VERSION "1.0"

class PeerInfo;
class Peer;
class CTCPClient;
class CTCPServer;
class DGMTPacket;
class DGMTProtocol;
class DGMTP2PProtocol;

class MindTrisClient
{
private:
	DGMTP2PProtocol * m_P2PProtocol;
	DGMTProtocol * m_Protocol;

	CTCPClient * m_Socket;
	CTCPServer * m_P2PSocket;

	queue<DGMTPacket *> m_Packets;
	vector<Peer *> * m_peers;
	vector<PeerInfo *> * m_peerinfovector;

	string m_displayname;
	bool m_DeleteMe;
	uint16_t m_listenport;
	uint32_t m_lobbyid;
	uint8_t m_peerid;
	uint64_t m_sessionid;
	bool m_inalobby;
	bool m_hasloggedin;
	string m_ErrorString;
	bool m_Error;

	RSAPublicKey * m_PublicKey;

	RSAPublicKey * m_serverPublicKey;
	CryptoPP::RSAES_OAEP_SHA_Encryptor * m_Encryptor;

public:
	void UpdateLeaveLobby();

	uint8_t GetPeerID(){ return m_peerid;}
	uint32_t GetLobbyID(){ return m_lobbyid;}
	vector<PeerInfo *> * GetPeerInfoVector() { return m_peerinfovector;}


	bool ServerStatusUpdate(fd_set * fd);
	void StopP2PSocket();
	void StartP2PSocket();
	vector<Peer *> * GetPeers() { return m_peers;}
	RSAPublicKey * GetServerPublicKey(){return m_serverPublicKey;}
    void PrintMalformedPacket();
	string GetDisplayName(){ return m_displayname;}
	void SetDisplayName( string s){ m_displayname = s;}
	void SetHasLoggedIn(bool b){ m_hasloggedin = b;}
	CryptoPP::RSAES_OAEP_SHA_Encryptor * GetEncryptor() { return m_Encryptor;}
	void RegisterUser(string username, string displayname, string email, string password);
	void RetrieveLobbyList();
	void CreateLobby(string lobbyname, uint8_t maxplayers,bool haspassword,string password);
	void JoinLobby(uint32_t lobbyid, string password);
	void LeaveLobby();
	void SetInALobby(bool b){ m_inalobby = b;}
	bool InALobby(){ return m_inalobby;}
	void Login(string username, string password);
	void SendChatCommand(string message);
	string GetVersionS(){ return MINDTRISCLIENT_VERSION;}
	bool HasLoggedIn(){ return m_hasloggedin;}
	MindTrisClient(string address, uint16_t port, uint16_t clientport);
	bool Update(long usecBlock);
	~MindTrisClient();
};

void CONSOLE_Print(string s);

#endif //MT_MINDTRIS_H