#ifndef MT_LOBBY_H
#define MT_LOBBY_H

class User;
typedef OrderedAllocationVector<User> UserList;



class Lobby
{
private:
	uint32_t m_id;
	uint64_t m_sessionid;

	User * m_creator;
	UserList * m_userlist;
	int m_maxplayers;
	string m_lobbyname;
	int m_playercount;
	string m_password;
	bool m_haspassword;

public:
	UserList * GetPeers(){ return m_userlist;}
	void SetSessionID(uint64_t sessionid){	m_sessionid = sessionid;	}
	void SetLobbyID(uint32_t id){	m_id = id;}
	string GetLobbyName(){		return m_lobbyname;	}
	string GetPassword(){		return m_password;	}
	int GetPlayerCount(){		return m_playercount;	}
	int GetMaxPlayers(){		return m_maxplayers;	}
	bool GetHasPassword(){		return m_haspassword;	}
	uint32_t GetLobbyID(){		return m_id;	}
	uint64_t GetSessionID(){	return m_sessionid;	}
	User * GetCreator(){		return m_creator;	}
	bool UserLeave(User * u);
	bool UserJoin(User * u, string password, DGMTProtocol::JoinedLobby * answer);
	Lobby(User * creator,string lobbyname, int maxplayers, bool haspassword, string password)
	{
		m_userlist = new UserList();
		m_creator = creator;
		m_lobbyname = lobbyname;
		m_playercount = 0;
		m_maxplayers = maxplayers;
		m_haspassword = haspassword;
		m_password = password;
	}
	Lobby::~Lobby();
};

#endif //MT_LOBBY_H