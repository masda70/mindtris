
#include "mindtris.h"
#include <locale>
#include <signal.h>
#include <stdlib.h>

#ifdef WIN32
 #include <ws2tcpip.h>		// for WSAIoctl
#endif

#ifdef WIN32
 #include <windows.h>
 #include <winsock.h>
#endif

#include <time.h>

#ifndef WIN32
 #include <sys/time.h>
#endif

#ifdef __APPLE__
 #include <mach/mach_time.h>
#endif


MindTrisClient::MindTrisClient(string address, uint16_t port){

	CONSOLE_Print("[MindTris++] Attempting to connect to server at ["+address+"] on port "+ UTIL_ToString(port));
	m_Socket = new CTCPClient( );
	m_Error=false;
	m_Protocol = new DGMTProtocol(true);
	CONSOLE_Print("[MindTris++] Connecting...");
	m_Socket-> Connect( "", address, port );


	m_Socket->CheckConnect( );
	CONSOLE_Print("[MindTris++] Connected!");

	fd_set fd;
	FD_ZERO( &fd );
	m_Socket->SetFD(&fd);
	CONSOLE_Print("[MindTris++] Sending Hello from Client");
	m_Socket->PutBytes(m_Protocol->SEND_DGMT_HELLOFROMCLIENT());
	m_Socket->DoSend(&fd);
	
	CONSOLE_Print("[MindTris++] Awaiting reply...");

	while(1)
	{
	FD_ZERO( &fd );
	m_Socket->SetFD(&fd);
		m_Socket->DoRecv(&fd);
		string *RecvBuffer = m_Socket->GetBytes( );
		BYTEARRAY Bytes = UTIL_CreateByteArray( (unsigned char *)RecvBuffer->c_str( ), RecvBuffer->size( ) );
		while( Bytes.size( ) >= m_Protocol->DGMT_MINIMALPACKETLENGTH )
		{
			DGMTPacket * packet = NULL;
			if(m_Protocol->ExtractPacket(Bytes, &packet))
			{
				if(packet!=NULL)
				{
					uint16_t Length = packet->GetLength();
					DGMTConnectionReply * reply;
					reply = m_Protocol->RECEIVE_DGMT_HELLOFROMSERVER(packet->GetData());
					if(reply->Connected()){
						CONSOLE_Print("[MindTris++] Succesfully connected to Server. Message from server:");
						CONSOLE_Print(reply->GetMessage());
					}else{
						CONSOLE_Print("[MindTris++] " + reply->GetMessage());
					}
					delete reply;
					*RecvBuffer = RecvBuffer->substr( Length );
					Bytes = BYTEARRAY( Bytes.begin( ) + Length, Bytes.end( ) );
				}else
					break;
			}else{
				m_Error = true;
				m_ErrorString = "received invalid packet from server";
				break;
			}
		}
		if(m_Error)
		{
			CONSOLE_Print("[MindTris++] received invalid packet from server. Closing...");
			delete m_Socket;
			return;
		}
	}

}

int main(int argc, char ** argv)
{
    SetConsoleOutputCP(CP_UTF8);

	/*int bufferSize = WideCharToMultiByte(CP_UTF8, 0, s, -1, NULL, 0, NULL, NULL);

	char* m = new char[bufferSize]; 

	WideCharToMultiByte(CP_UTF8, 0, s, -1, m,bufferSize, NULL, NULL);  */

	#ifdef WIN32
		// initialize winsock

		CONSOLE_Print( "[MindTris++] starting winsock" );
		WSADATA wsadata;

		if( WSAStartup( MAKEWORD( 2, 2 ), &wsadata ) != 0 )
		{
			CONSOLE_Print( "[MindTris++] error starting winsock" );
			return 1;
		}
	#endif

 	switch(argc)
	{
		case 1: 
			MindTrisClient("localhost",1379);
			break;
		case 2:
			MindTrisClient("localhost",atoi(argv[1]));
			break;
		case 3:
			MindTrisClient(argv[1],atoi(argv[2]));
			break;
	}
}