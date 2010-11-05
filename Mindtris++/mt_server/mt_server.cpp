#include <mindtriscore\mindtriscore.h>

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

#define SERVERMOTD "Let's play some SC2"


#include "mt_server.h"


MindTrisServer :: MindTrisServer(string address, uint16_t port, string nMOTD)
{
	m_Socket = new CTCPServer(true);
	m_Protocol = new DGMTProtocol(true);
	m_BindAddress = address;
	m_ListenPort = port;
	m_Exiting = false;
	m_MOTD = nMOTD;

	if( m_Socket->Listen( m_BindAddress, m_ListenPort ) )
		CONSOLE_Print( "[MindTris Server] listening on port " + UTIL_ToString( m_ListenPort ) );
	else
	{
		CONSOLE_Print( "[MindTris Server] error listening on port " + UTIL_ToString( m_ListenPort ) );
		m_Exiting = true;
	}
}

MindTrisServer :: ~MindTrisServer(){
	delete m_Socket;
	delete m_Protocol;

}


bool MindTrisServer :: Update(long usecBlock){
	unsigned int NumFDs = 0;

	int nfds = 0;
	fd_set fd;
	fd_set send_fd;
	FD_ZERO( &fd );
	FD_ZERO( &send_fd );

	// User sockets

	for( vector<User *> :: iterator i = m_Users.begin( ); i != m_Users.end( ); i++ )
	{
		(*i)->GetSocket( )->SetFD( &fd, &send_fd, &nfds );
		NumFDs++;
	}

	// Server Socket

	if(m_Socket){
		m_Socket->SetFD( &fd, &send_fd, &nfds );
		NumFDs++;
	}

	if( usecBlock < 1000 )
		usecBlock = 1000;

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = usecBlock;

	struct timeval send_tv;
	send_tv.tv_sec = 0;
	send_tv.tv_usec = 0;

	#ifdef WIN32
		select( 1, &fd, NULL, NULL, &tv );
		select( 1, NULL, &send_fd, NULL, &send_tv );
	#else
		select( nfds + 1, &fd, NULL, NULL, &tv );
		select( nfds + 1, NULL, &send_fd, NULL, &send_tv );
	#endif

	if( NumFDs == 0 )
	{
		MILLISLEEP( 50 );
	}

	if( m_Socket )
	{
		CTCPSocket *NewSocket = m_Socket->Accept( &fd );

		if( NewSocket )
		{

				NewSocket->SetNoDelay( true );

				m_Users.push_back( new User( this, m_Protocol, NewSocket ) );

				CONSOLE_Print( "[MindTris Server] connection attempt from [" + NewSocket->GetIPString( ) + "]" );
		}

		if( m_Socket->HasError( ) )
		{
			CONSOLE_Print( "[MindTris Server] Socket error: " + m_Socket->GetErrorString()  );
			return true;
		}
	}

	for( vector<User *> :: iterator i = m_Users.begin( ); i != m_Users.end( );  )
	{

		if( (*i)->Update( &fd ) )
		{
			delete *i;
			i = m_Users.erase( i );
		}
		else
		{
			if( (*i)->GetSocket( ) )
				(*i)->GetSocket( )->DoSend( &send_fd );
			i++;
		}
	}

	return false;
}

int main(int argc, char ** argv)
{
	#ifdef WIN32
		// initialize winsock

		CONSOLE_Print( "[MindTris Server] starting winsock" );
		WSADATA wsadata;

		if( WSAStartup( MAKEWORD( 2, 2 ), &wsadata ) != 0 )
		{
			CONSOLE_Print( "[MindTris Server] error starting winsock" );
			return 1;
		}

		// increase process priority

		//CONSOLE_Print( "[MindTris Server] setting process priority to \"above normal\"" );
	//	SetPriorityClass( GetCurrentProcess( ), ABOVE_NORMAL_PRIORITY_CLASS );
	#endif

	MindTrisServer * server;

	// initialize server

	switch(argc)
	{
		case 1: 
			server = new MindTrisServer("",1379, SERVERMOTD);
			break;
		case 2:
			server = new MindTrisServer("",atoi(argv[1]), SERVERMOTD);
			break;
		case 3:
			server = new MindTrisServer(argv[1],atoi(argv[2]), SERVERMOTD);
			break;
	}


	while( 1 )
	{

		if( server->Update( 50000 ) )
			break;
	}

}


