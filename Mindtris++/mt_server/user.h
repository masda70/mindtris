#ifndef MT_USER_H
#define MT_USER_H

class Lobby;
class CTCPServer;
class CTCPSocket;
class DGMTProtocol;
class MindTrisServer;
class PacketExtractor;

class User : NonCopyable
{
private:
	int m_id;

	
	uint8_t m_peerid; //peer id in the lobby-game
	Lobby * m_lobby;
	uint16_t m_p2pportnumber;
	DSAPublicKey m_publickey;
	string m_displayname;

	MessageStreamer m_MessageStreamer;
	unique_ptr<Packet> m_IncompletePacket;

	MindTrisServer & m_Server;
	unique_ptr<CTCPSocket> m_Socket;

	string m_ErrorString;


	DGMTProtocol & m_Protocol;
	queue<Message> m_Messages;

	bool m_loadedgame;

	bool m_Error;
	bool m_DeleteMe;

	void PrintMalformedMessage();

public:

	void User::Send( Message && msg );
	bool Update(fd_set * fd);

	bool HasLoadedGame() { return m_loadedgame;}
	void SetHasLoadedGame( bool b) { m_loadedgame = b;}

	void SetDisplayName(string s){ m_displayname =s;}

	void SendPeerStatusUpdate(User &, DGMTProtocol::UpdateClientStatus::status);

	string GetDisplayName() const{ return m_displayname;}
	DSAPublicKey GetPublicKey() const{ return m_publickey;}
	uint16_t GetP2PPortNumber() const { return m_p2pportnumber;}
	Lobby * GetLobby() { return m_lobby; }
	const Lobby * GetLobby() const{ return m_lobby; }
	uint8_t GetPeerID() const{ return m_peerid; }
	uint32_t GetIPInt32() const;
	const unique_ptr<CTCPSocket> & GetSocket( ) const { return m_Socket; }

	void SetPublicKey(DSAPublicKey s){ m_publickey = s;}
	void SetP2PPortNumber(uint16_t p){ m_p2pportnumber= p;}
	void SetLobby(Lobby * l){ m_loadedgame = false; m_lobby = l; }
	void SetPeerID(uint8_t id){ m_peerid = id;}


	User(MindTrisServer & nServer, DGMTProtocol & nProtocol, unique_ptr<CTCPSocket> nSocket);
	~User();
};

#endif // MT_USER_H