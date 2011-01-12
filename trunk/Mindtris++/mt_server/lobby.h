#ifndef MT_LOBBY_H
#define MT_LOBBY_H

class User;

typedef OrderedAllocationVector<User *> UserList;



class Lobby : NonCopyable
{
private:
	uint32_t m_id;
	uint64_t m_sessionid;
	
	uint32_t m_piecesoffset;


	int m_maxplayers;
	string m_lobbyname;
	int m_playercount;
	string m_password;
	bool m_haspassword;
	bool m_gamestarting;
	bool m_gamestarted;

	User & m_creator;
	UserList m_userlist;

public:
	vector<uint8_t> GetNextPieces( CryptoPP::RandomNumberGenerator & rng, uint8_t number);
	void SetGameStarting(bool b){ m_gamestarting = b;}
	bool GetGameStarting() const{ return m_gamestarting;}

	void SetGameStarted(bool b) { m_gamestarted = b;}
	bool GetGameStarted() const { return m_gamestarted;}
	uint32_t GetPieceOffset() const { return m_piecesoffset;}

	UserList & GetPeers(){ return m_userlist;}
	void SetSessionID(uint64_t sessionid){	m_sessionid = sessionid;	}
	void SetLobbyID(uint32_t id){	m_id = id;}
	string GetLobbyName() const{		return m_lobbyname;	}
	string GetPassword() const{		return m_password;	}
	int GetPlayerCount()const{		return m_playercount;	}
	int GetMaxPlayers() const{		return m_maxplayers;	}
	bool GetHasPassword() const{		return m_haspassword;	}
	uint32_t GetLobbyID() const{		return m_id;	}
	uint64_t GetSessionID() const{	return m_sessionid;	}
	const User & GetCreator() const{		return m_creator;	}
	bool UserLeave(User & u);
	bool UserJoin(User & u, string password, DGMTProtocol::JoinedLobby::answer & answer);
	Lobby(User & creator,string lobbyname, int maxplayers, bool haspassword, string password):
		m_creator(creator)
	{
		m_piecesoffset = 0;
		m_gamestarted = false;
		m_gamestarting = false;
		m_lobbyname = lobbyname;
		m_playercount = 0;
		m_maxplayers = maxplayers;
		m_haspassword = haspassword;
		m_password = password;
	}
	~Lobby();
};

#endif //MT_LOBBY_H