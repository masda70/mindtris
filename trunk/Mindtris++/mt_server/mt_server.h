#ifndef MT_SERVER_H
#define MT_SERVER_H

#include <mindtriscore\mindtriscore.h>
#include "user.h"

class MindTrisServer
{
private:
	string m_MOTD;
	vector<User *> m_Users;
	DGMTProtocol * m_Protocol;
	CTCPServer * m_Socket;
	string m_BindAddress;
	uint16_t m_ListenPort;
	bool m_Exiting;

public:
	string GetMOTD() { return m_MOTD; };
	MindTrisServer(string address, uint16_t port, string nMOTD);
	~MindTrisServer();
	MindTrisServer(){}
	
	bool Update(long usecBlock);
};

#endif