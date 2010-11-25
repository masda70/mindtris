#include "mindtriscore/includes.h"
#include "mindtriscore/util.h"
#include "mindtriscore/socket.h"
#include "mindtriscore/oalloc.h"
#include "mindtriscore/serverprotocol.h"
#include "mindtriscore/p2pprotocol.h"
#include "lobby.h"
#include "user.h"
#include "database.h"
#include "mt_server.h"


#define SERVERMOTD "Welcome to MasdaSeventy's MindTris++ Server."




void CONSOLE_Print( string message )
{
	cout << message << endl;
}

void DEBUG_Print( string message )
{
	cout << message << endl;
}

void DEBUG_Print( BYTEARRAY b )
{
	cout << "{ ";

	for( unsigned int i = 0; i < b.size( ); i++ )
		cout << hex << (int)b[i] << " ";

	cout << "}" << endl;
}

void MindTrisServer :: DestroyLobby(Lobby * l)
{
	m_Lobbies->remove(l->GetLobbyID());
	delete l;
}

Lobby * MindTrisServer :: CreateLobby(User * creator, string lobbyname, int maxplayers, bool haspassword, string password)
{
	Lobby * l = new Lobby(creator, lobbyname, maxplayers, haspassword, password);
	l->SetLobbyID(m_Lobbies->add(l));
	l->SetSessionID( ((uint64_t) m_randPool.GenerateByte()) << 56 |((uint64_t) m_randPool.GenerateByte()) << 48 | ((uint64_t) m_randPool.GenerateByte()) << 40 | ((uint64_t) m_randPool.GenerateByte()) <<  32 | ((uint64_t) m_randPool.GenerateByte()) << 24 | ((uint64_t) m_randPool.GenerateByte()) << 16 | ((uint64_t) m_randPool.GenerateByte()) << 8| ((uint64_t) m_randPool.GenerateByte()));
	return l;
}

MindTrisServer :: MindTrisServer(string address, uint16_t port, string nMOTD)
{

	m_Users= new vector<User *>;
	m_Lobbies = new OrderedAllocationVector<Lobby>();

	m_Socket = new CTCPServer(&CONSOLE_Print,true);
	m_Protocol = new DGMTProtocol(true);
	m_BindAddress = address;
	m_ListenPort = port;
	m_Exiting = false;
	m_MOTD = nMOTD;

	m_database = new ServerDatabase();

	CryptoPP::InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(m_randPool, 3072);

	CryptoPP::Integer modulus = params.GetModulus();
	CryptoPP::Integer exponent = params.GetPublicExponent();

	m_Decryptor = new CryptoPP::RSAES_OAEP_SHA_Decryptor(params);
		
	string modulusstring;
	CryptoPP::TransparentFilter modulusFilter(new CryptoPP::StringSink(modulusstring));
	modulus.Encode(modulusFilter,modulus.MinEncodedSize(),CryptoPP::Integer::UNSIGNED);

	string exponentstring;
	CryptoPP::TransparentFilter exponentFilter(new CryptoPP::StringSink(exponentstring));
	exponent.Encode(exponentFilter,exponent.MinEncodedSize(),CryptoPP::Integer::UNSIGNED);

	m_PublicKey = new RSAPublicKey();
	m_PublicKey->Exponent = exponentstring;
	m_PublicKey->Modulus = modulusstring;


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

	for( vector<User *> :: iterator i = m_Users->begin( ); i != m_Users->end( ); i++ )
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

				m_Users->push_back( new User( this, m_Protocol, NewSocket ) );

				CONSOLE_Print( "[MindTris Server] connection attempt from [" + NewSocket->GetIPString( ) + "]" );
		}

		if( m_Socket->HasError( ) )
		{
			CONSOLE_Print( "[MindTris Server] Socket error: " + m_Socket->GetErrorString()  );
			return true;
		}
	}

	for( vector<User *> :: iterator i = m_Users->begin( ); i != m_Users->end( );  )
	{

		if( (*i)->Update( &fd ) )
		{
			delete *i;
			i = m_Users->erase( i );
		}
		else i++;
	}

	for( vector<User *> :: iterator i = m_Users->begin( ); i != m_Users->end( );  i++)
	{
		if( (*i)->GetSocket( ) ) (*i)->GetSocket( )->DoSend( &send_fd );
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


