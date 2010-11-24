#ifndef MT_USER_H
#define MT_USER_H

#include "mt_server.h"

class MindTrisServer;

class User
{
private:
	int m_id;

	uint8_t m_peerid; //peer id in the lobby-game
	Lobby * m_lobby;
	uint16_t m_p2pportnumber;
	RSAPublicKey * m_publickey;
	string m_displayname;

	MindTrisServer * m_Server;
	string m_ErrorString;
	CTCPSocket *m_Socket;
	DGMTProtocol *m_Protocol;
	queue<DGMTPacket *> m_Packets;

	bool m_Error;
	bool m_DeleteMe;

	void PrintMalformedPacket();

public:

    void Send( BYTEARRAY data );
	bool Update(fd_set * fd);

	void SetDisplayName(string s){ m_displayname =s;}
	string GetDisplayName(){ return m_displayname;}
	bool JoinLobby(Lobby * lobby, string password, DGMTProtocol::JoinedLobby * answer);
	bool LeaveLobby();
	
	void SendPeerStatusUpdate(User *, DGMTProtocol::StatusUpdate);
	void SetPublicKey(RSAPublicKey * s){ m_publickey = s;}
	RSAPublicKey * GetPublicKey(){ return m_publickey;}
	void SetP2PPortNumber(uint16_t p){ m_p2pportnumber= p;}
	uint16_t GetP2PPortNumber() { return m_p2pportnumber;}
	uint32_t GetIPInt32(){return m_Socket->GetIPInt32();}
	
	void SetLobby(Lobby * l){ m_lobby = l; }
	Lobby * GetLobby(){ return m_lobby; }
	uint8_t GetPeerID(){ return m_peerid; }
	void SetPeerID(uint8_t id){ m_peerid = id;}

	CTCPSocket *GetSocket( ) { return m_Socket; }
	User(MindTrisServer * nServer, DGMTProtocol * nProtocol, CTCPSocket * nSocket);
	~User();
};

#endif // MT_USER_H