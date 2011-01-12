#ifndef MT_SERVER_H
#define MT_SERVER_H

#include "mindtriscore/includes.h"

class Lobby;
class User;
class CTCPServer;
class DGMTProtocol;
template <typename T> class OrderedAllocationVector;

class ServerDatabase;

typedef OrderedAllocationVector<Lobby *> LobbyList;

class MindTrisServer : NonCopyable
{
private:
	string m_MOTD;
	CryptoPP::AutoSeededRandomPool m_randPool;

	vector<unique_ptr<User>> m_Users;
	LobbyList m_Lobbies;

	DGMTProtocol m_Protocol;
	unique_ptr<CTCPServer> m_Socket;
	string m_BindAddress;
	uint16_t m_ListenPort;
	bool m_Exiting;

	ServerDatabase m_database;

	RSAPublicKey m_PublicKey;
	unique_ptr<CryptoPP::RSAES_OAEP_SHA_Decryptor> m_Decryptor;

public:
	 CryptoPP::RandomNumberGenerator & GetRandomGenerator() { return m_randPool;}
	const unique_ptr<CryptoPP::RSAES_OAEP_SHA_Decryptor> & GetDecryptor() { return m_Decryptor;}
	ServerDatabase & GetDatabase() { return m_database;}
	LobbyList & GetLobbies() { return m_Lobbies;}
	const RSAPublicKey & GetPublicKey(){ return m_PublicKey;}

	string GetMOTD() { return m_MOTD; };
	MindTrisServer(string address, uint16_t port, string nMOTD);

	~MindTrisServer();
	MindTrisServer(){}
	
	void DestroyLobby(Lobby * l);
	Lobby * CreateLobby(User & creator, string lobbyname, int maxplayercount, bool haspassword, string password);
	bool Update(long usecBlock);
};

// output
void CONSOLE_Print( string message );
void DEBUG_Print( string message );
void DEBUG_Print( const ByteArray &b );


#endif


