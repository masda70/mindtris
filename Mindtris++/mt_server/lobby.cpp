
#include "mindtriscore/includes.h"
#include "mindtriscore/util.h"
#include "mindtriscore/bytearray.h"
#include "mindtriscore/bytebuffer.h"
#include "mindtriscore/socket.h"
#include "mindtriscore/oalloc.h"
#include "mindtriscore/commprotocol.h"
#include "mindtriscore/packet.h"
#include "mindtriscore/messagestreamer.h"
#include "mindtriscore/serverprotocol.h"
#include "user.h"
#include "lobby.h"

Lobby:: ~Lobby(){
	for (UserList::iterator it = GetPeers().begin(); it != GetPeers().end(); it++) 
	{
		User & peer = *it->second;
		peer.SendPeerStatusUpdate(peer,DGMTProtocol::UpdateClientStatus::STATUSUPDATE_HASLEFTTHELOBBY);
		peer.SetLobby(nullptr);
	}
}

vector<uint8_t> Lobby::GetNextPieces(CryptoPP::RandomNumberGenerator & rng,uint8_t number){
	unsigned int n = (number-1)/7 + 1;
	vector<uint8_t> pieces;
	for(int j = 0; j<n; j++)
	{
		uint8_t	perm[] = {0,1,2,3,4,5,6};
		for(int i = 0; i<6; i++)
		{
			unsigned int r = rng.GenerateWord32(i,6);
			pieces.push_back(perm[r]);
			perm[r] = perm[i];
		}
		pieces.push_back(perm[6]);
	}
	m_piecesoffset += pieces.size();
	return pieces;
}

bool Lobby::UserLeave(User & u)
{

	if(u.GetLobby() == this)
	{
		for (UserList::iterator it = GetPeers().begin(); it != GetPeers().end(); it++) 
		{
			User & peer = *it->second;
			peer.SendPeerStatusUpdate(u,DGMTProtocol::UpdateClientStatus::STATUSUPDATE_HASLEFTTHELOBBY);
		}
		m_playercount--;
		GetPeers().remove(u.GetPeerID());
		return true;
	}
	return false;
}


bool Lobby::UserJoin(User & u, string password, DGMTProtocol::JoinedLobby::answer & answer)
{
	if(m_gamestarting){
		answer = DGMTProtocol::JoinedLobby::JOINEDLOBBY_UNKNOWNERROR; return false;
	}

	if(m_haspassword && password != m_password)
	{
		answer = DGMTProtocol::JoinedLobby::JOINEDLOBBY_WRONGPASSWORD; return false;
	}
	if(u.GetLobby() != NULL)
	{
		answer = DGMTProtocol::JoinedLobby::JOINEDLOBBY_UNKNOWNERROR; return false;
	}
	if(m_maxplayers > m_playercount)
	{
		u.SetPeerID(m_userlist.add(&u));
		m_playercount++;
		answer = DGMTProtocol::JoinedLobby::JOINEDLOBBY_SUCCESS; return true;
	}else{
		answer = DGMTProtocol::JoinedLobby::JOINEDLOBBY_LOBBYFULL; return false;
	}
}

//m_userlist.remove(u->GetPeerID());