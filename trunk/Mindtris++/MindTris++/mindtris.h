#ifndef MINDTRIS_H
#define MINDTRIS_H

#include <mindtriscore\mindtriscore.h>

class MindTrisClient
{
private:
	DGMTProtocol * m_Protocol;
	CTCPClient * m_Socket;
	string m_ErrorString;
	bool m_Error;
public:
	MindTrisClient(string address, uint16_t port);
};
#endif //MINDTRIS_H