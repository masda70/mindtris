#include "mt_server.h"

Lobby:: ~Lobby(){
	for (OrderedAllocationVector<User>::iterator it = GetPeers()->begin(); it != GetPeers()->end(); it++) 
	{
		User * peer = it->second;
		peer->SendPeerStatusUpdate(peer,DGMTProtocol::STATUSUPDATE_HASLEFTTHELOBBY);
		peer->LeaveLobby();
	}
}

bool Lobby::UserLeave(User * u)
{
	if(u->GetLobby() == this)
	{
		for (OrderedAllocationVector<User>::iterator it = GetPeers()->begin(); it != GetPeers()->end(); it++) 
		{
			User * peer = it->second;
			peer->SendPeerStatusUpdate(u,DGMTProtocol::STATUSUPDATE_HASLEFTTHELOBBY);
		}
		m_playercount--;
		m_userlist->remove(u->GetPeerID());
		return true;
	}
	return false;
}


bool Lobby::UserJoin(User * u, string password, DGMTProtocol::JoinedLobby * answer)
{
	if(m_haspassword && password != m_password)
	{
		*answer = DGMTProtocol::JOINEDLOBBY_WRONGPASSWORD; return false;
	}
	if(u->GetLobby() != NULL)
	{
		*answer = DGMTProtocol::JOINEDLOBBY_UNKNOWNERROR; return false;
	}
	if(m_maxplayers > m_playercount)
	{
		u->SetPeerID(m_userlist->add(u));
		m_playercount++;
		*answer = DGMTProtocol::JOINEDLOBBY_SUCCESS; return true;
	}else{
		*answer = DGMTProtocol::JOINEDLOBBY_LOBBYFULL; return false;
	}
}

//m_userlist.remove(u->GetPeerID());