#ifndef MT_SERVER_H
#define MT_SERVER_H

#include <mindtriscore\mindtriscore.h>
#include "lobby.h"
#include "user.h"
#include "database.h"

typedef OrderedAllocationVector<Lobby> LobbyList;

class MindTrisServer
{
private:
	string m_MOTD;
	CryptoPP::AutoSeededRandomPool m_randPool;

	vector<User *> * m_Users;
	OrderedAllocationVector<Lobby> * m_Lobbies;

	DGMTProtocol * m_Protocol;
	CTCPServer * m_Socket;
	string m_BindAddress;
	uint16_t m_ListenPort;
	bool m_Exiting;

	ServerDatabase * m_database;

	RSAPublicKey * m_PublicKey;
	CryptoPP::RSAES_OAEP_SHA_Decryptor * m_Decryptor;

public:
	CryptoPP::RSAES_OAEP_SHA_Decryptor * GetDecryptor() { return m_Decryptor;}
	ServerDatabase * GetDatabase() { return m_database;}
	OrderedAllocationVector<Lobby> * GetLobbies() { return m_Lobbies;}
	RSAPublicKey * GetPublicKey(){ return m_PublicKey;}
	string GetMOTD() { return m_MOTD; };
	MindTrisServer(string address, uint16_t port, string nMOTD);
	~MindTrisServer();
	MindTrisServer(){}
	
	void MindTrisServer :: DestroyLobby(Lobby * l);
	Lobby * CreateLobby(User * creator, string lobbyname, int maxplayercount, bool haspassword, string password);
	bool Update(long usecBlock);
};

// output
void CONSOLE_Print( string message );
void DEBUG_Print( string message );
void DEBUG_Print( BYTEARRAY b );


#endif


