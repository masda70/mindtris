

#ifndef MT_PEER_H
#define MT_PEER_H



class CTCPClient;
class CTCPServer;
class Packet;
class DGMTProtocol;
class DGMTP2PProtocol;

class MindTrisClient;

class PeerInfo
{
private:
	uint8_t m_peerid;
	string m_displayname;
	uint32_t m_ipaddress;
	uint16_t m_portnumber;
	DSAPublicKey m_publickey;
public:
	uint8_t GetPeerID(){ return m_peerid;}
	uint32_t GetIPAddress(){ return m_ipaddress;}
	uint16_t GetPortNumber(){return m_portnumber;}
	string GetDisplayName(){ return m_displayname;}
	const DSAPublicKey & GetPublicKey() { return m_publickey;}
	PeerInfo(uint8_t peerid, string displayname, uint32_t ipaddress, uint16_t portnumber, const DSAPublicKey & key){
		m_peerid = peerid;
		m_displayname = displayname;
		m_ipaddress = ipaddress;
		m_portnumber = portnumber;
		m_publickey = key;
	}
	~PeerInfo(){}

};

class Peer : private NonCopyable
{
public:
	enum ConnectionStatus{
		STATUS_LISTENING,
		STATUS_CONNECTIONREQUEST,
		STATUS_CONNECTIONACCEPTED,
		STATUS_CONNECTED
	};
private:
	unique_ptr<Tetris> m_tetris;

	ConnectionStatus m_status;

	bool m_Error;
	bool m_DeleteMe;
	
	uint64_t m_challengeforpeer;
	uint64_t m_challengefrompeer;

	uint8_t m_id;
	uint8_t m_peerid; 
	CTCPClient * m_ClientSocket; /* If the client initiated the connection, this is not null */
	unique_ptr<CTCPSocket> m_Socket;

	string m_ErrorString;
	string m_displayname;
	DSAPublicKey m_publickey;
	MindTrisClient & m_client;
	DGMTP2PProtocol & m_Protocol;
	
	MessageStreamer m_MessageStreamer;
	queue<Message> m_Messages;
	unique_ptr<Packet> m_IncompletePacket;

	unique_ptr<CryptoPP::DSA::Verifier> m_Verifier;

public:
	void SendRoundData(uint64_t sessionid, uint32_t round, vector<DGMTP2PProtocol::Move> moves, const unique_ptr<CryptoPP::DSA::Signer> & signer);
	void NewGame(vector<uint8_t> pieces);
	Tetris & GetGame(){ return *m_tetris;}
	const unique_ptr<CryptoPP::DSA::Verifier> & GetVerifier() const{return m_Verifier;}

	bool InitializeVerifier(const DSAPublicKey & key);

	void Send( Message && msg );
	bool Update(fd_set * fd);
	void SendChat(uint64_t sessionid, const string & msg, const unique_ptr<CryptoPP::DSA::Signer> & signer);

	void SetConnectionStatus(ConnectionStatus s){ m_status = s;}
	ConnectionStatus GetConnectionStatus() const { return m_status;}

	void SetChallengeFromPeer(uint64_t c) { m_challengefrompeer = c;}
	uint64_t GetChallengeFromPeer() const { return m_challengefrompeer;}

	void SetChallengeForPeer(uint64_t c) { m_challengeforpeer = c;}
	uint64_t GetChallengeForPeer() const { return m_challengeforpeer;}

	string GetDisplayName(){ return m_displayname;}
	void SetDisplayName(string s){ m_displayname = s;}

	void SetPublicKey(const DSAPublicKey & s){ m_publickey = s;}
	const DSAPublicKey & GetPublicKey(){ return m_publickey;}


	uint8_t GetPeerID(){return m_peerid;}
	void SetPeerID(uint8_t id){ m_peerid = id;}

	void PrintMalformedMessage();


	CTCPClient * GetClientSocket(){ return m_ClientSocket;}
	unique_ptr<CTCPSocket> & GetSocket( ) { return m_Socket; }
	Peer(MindTrisClient & client, DGMTP2PProtocol & protocol, unique_ptr<CTCPSocket> & nSocket);
    Peer(MindTrisClient & client,  DGMTP2PProtocol & protocol, uint8_t peerid, const string & displayname, uint32_t ipaddress, uint16_t port, const DSAPublicKey & publickey);
	~Peer();
};

#endif //MT_PEER_H