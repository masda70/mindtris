#ifndef USER_H
#define USER_H

#include "mindtriscore\mindtriscore.h"

class MindTrisServer;

class User
{
private:
	MindTrisServer * m_Server;
	bool m_Error;
	bool m_DeleteMe;
	string m_ErrorString;
	CTCPSocket *m_Socket;
	DGMTProtocol *m_Protocol;
	queue<DGMTPacket *> m_Packets;

public:
    void Send( BYTEARRAY data );
	bool Update(fd_set * fd);
	CTCPSocket *GetSocket( ) { return m_Socket; }
	User(MindTrisServer * nServer, DGMTProtocol * nProtocol, CTCPSocket * nSocket);
	~User();
};

#endif // USER_H