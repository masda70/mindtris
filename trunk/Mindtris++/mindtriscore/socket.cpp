#include "mindtriscore.h"
#include "util.h"
#include "socket.h"

#include <string.h>

#ifndef WIN32
 int GetLastError( ) { return errno; }
#endif

//
// CSocket
//

CSocket :: CSocket( void (*console_print)(string) )
{
	m_CONSOLEPrint = console_print;
	m_Socket = INVALID_SOCKET;
	memset( &m_SIN, 0, sizeof( m_SIN ) );
	m_HasError = false;
	m_Error = 0;
	m_IsBlocking = false;
}

CSocket :: CSocket(void (*console_print)(string), SOCKET nSocket, struct sockaddr_in nSIN )
{
	m_CONSOLEPrint = console_print;
	m_Socket = nSocket;
	m_SIN = nSIN;
	m_HasError = false;
	m_Error = 0;
	m_IsBlocking = false;
}

CSocket :: ~CSocket( )
{
	if( m_Socket != INVALID_SOCKET )
		closesocket( m_Socket );
}

BYTEARRAY CSocket :: GetPort( )
{
	return UTIL_CreateByteArray( m_SIN.sin_port, false );
}

BYTEARRAY CSocket :: GetIP( )
{
	return UTIL_CreateByteArray( (uint32_t)m_SIN.sin_addr.s_addr, false );
}

uint32_t CSocket :: GetIPInt32()
{
	return (uint32_t)m_SIN.sin_addr.s_addr;
}

string CSocket :: GetIPString( )
{
	return inet_ntoa( m_SIN.sin_addr );
}

string CSocket :: GetErrorString( )
{
	if( !m_HasError )
		return "NO ERROR";

	return "ERROR (" + UTIL_ToString( m_Error ) + ")";
}

void CSocket :: SetFD( fd_set *fd )
{
	if( m_Socket == INVALID_SOCKET )
		return;

	FD_SET( m_Socket, fd );

}

void CSocket :: SetFD( fd_set *fd, fd_set *send_fd, int *nfds )
{
	if( m_Socket == INVALID_SOCKET )
		return;

	FD_SET( m_Socket, fd );
	FD_SET( m_Socket, send_fd );

#ifndef WIN32
	if( m_Socket > *nfds )
		*nfds = m_Socket;
#endif
}

void CSocket :: Allocate( int type )
{
	m_Socket = socket( AF_INET, type, 0 );

	if( m_Socket == INVALID_SOCKET )
	{
		m_HasError = true;
		m_Error = GetLastError( );
		(*m_CONSOLEPrint)( "[SOCKET] error (socket) - " + GetErrorString( ) );
		return;
	}
}

void CSocket :: Reset( )
{
	if( m_Socket != INVALID_SOCKET )
		closesocket( m_Socket );

	m_Socket = INVALID_SOCKET;
	memset( &m_SIN, 0, sizeof( m_SIN ) );
	m_HasError = false;
	m_Error = 0;
}

void CSocket :: SetBlocking(bool nIsBlocking)
{
	m_IsBlocking = nIsBlocking;
	if(m_IsBlocking)
	{
	#ifdef WIN32
		int iMode = 1;
		ioctlsocket( m_Socket, FIONBIO, (u_long FAR *)&iMode );
	#else
		fcntl( m_Socket, F_SETFL, fcntl( m_Socket, F_GETFL ) | O_NONBLOCK );
	#endif
	}
}

//
// CTCPSocket
//

CTCPSocket :: CTCPSocket( void (*console_print)(string)) : CSocket( console_print)
{
	Allocate( SOCK_STREAM );
	m_Connected = false;
}

CTCPSocket :: CTCPSocket(void (*console_print)(string),  SOCKET nSocket, struct sockaddr_in nSIN ) : CSocket(console_print,  nSocket, nSIN )
{
	m_Connected = true;
	SetBlocking(m_IsBlocking);
}

CTCPSocket :: ~CTCPSocket( )
{

}

void CTCPSocket :: Reset( )
{
	CSocket :: Reset( );

	Allocate( SOCK_STREAM );
	m_Connected = false;
	m_RecvBuffer.clear( );
	m_SendBuffer.clear( );

}

void CTCPSocket :: PutBytes( BYTEARRAY bytes )
{
	m_SendBuffer.append(string(bytes.begin(),bytes.end()));
}

void CTCPSocket :: DoRecv( )
{

		char buffer[1024];
		int c = recv( m_Socket, buffer, 1024, 0 );

		if( c == SOCKET_ERROR && GetLastError( ) != EWOULDBLOCK )
		{
			// receive error

			m_HasError = true;
			m_Error = GetLastError( );
			(*m_CONSOLEPrint)( "[TCPSOCKET] error (recv) - " + GetErrorString( ) );
			return;
		}
		else if( c == 0 )
		{
			// the other end closed the connection

			(*m_CONSOLEPrint)( "[TCPSOCKET] closed by remote host" );
			m_Connected = false;
		}
		else if( c > 0 )
		{
			// success! add the received data to the buffer

			m_RecvBuffer.append(buffer,c);
	
		//	m_LastRecv = GetTime( );
		}
}
void CTCPSocket :: DoRecv( fd_set *fd )
{
	if( m_Socket == INVALID_SOCKET || m_HasError || !m_Connected )
		return;

	if( FD_ISSET( m_Socket, fd ) )
	{
		// data is waiting, receive it
		DoRecv();
	}
}

void CTCPSocket :: DoSend( )
{

	int s = send( m_Socket, m_SendBuffer.data(), (int) m_SendBuffer.size( ), MSG_NOSIGNAL );

	if( s == SOCKET_ERROR && GetLastError( ) != EWOULDBLOCK )
	{
		// send error

		m_HasError = true;
		m_Error = GetLastError( );
		(*m_CONSOLEPrint)( "[TCPSOCKET] error (send) - " + GetErrorString( ) );
		return;
	}
	else if( s > 0 )
	{
		// success! only some of the data may have been sent, emove it from the buffer
		m_SendBuffer = m_SendBuffer.substr(s);
		//m_LastSend = GetTime( );
	}
}

void CTCPSocket :: DoSend( fd_set *send_fd )
{
	if( m_Socket == INVALID_SOCKET || m_HasError || !m_Connected || m_SendBuffer.empty( ) )
		return;

	if( FD_ISSET( m_Socket, send_fd ) )
	{
		// socket is ready, send it
		DoSend();		
	}
}

void CTCPSocket :: Disconnect( )
{
	if( m_Socket != INVALID_SOCKET )
		shutdown( m_Socket, SHUT_RDWR );

	m_Connected = false;
}

void CTCPSocket :: SetNoDelay( bool noDelay )
{
	int OptVal = 0;

	if( noDelay )
		OptVal = 1;

	setsockopt( m_Socket, IPPROTO_TCP, TCP_NODELAY, (const char *)&OptVal, sizeof( int ) );
}

//
// CTCPClient
//

CTCPClient :: CTCPClient( void (*console_print)(string), bool nIsBlocking) : CTCPSocket( console_print )
{
	SetBlocking(nIsBlocking);
	m_Connecting = false;
}

CTCPClient :: ~CTCPClient( )
{

}

void CTCPClient :: Reset( )
{
	CTCPSocket :: Reset( );
	m_Connecting = false;
}

void CTCPClient :: Disconnect( )
{
	CTCPSocket :: Disconnect( );
	m_Connecting = false;
}

void CTCPClient :: Connect( string localaddress, uint32_t HostAddress, uint16_t port )
{
	if( m_Socket == INVALID_SOCKET || m_HasError || m_Connecting || m_Connected )
		return;

	if( !localaddress.empty( ) )
	{
		struct sockaddr_in LocalSIN;
		memset( &LocalSIN, 0, sizeof( LocalSIN ) );
		LocalSIN.sin_family = AF_INET;

		if( ( LocalSIN.sin_addr.s_addr = inet_addr( localaddress.c_str( ) ) ) == INADDR_NONE )
			LocalSIN.sin_addr.s_addr = INADDR_ANY;

		LocalSIN.sin_port = htons( 0 );

		if( bind( m_Socket, (struct sockaddr *)&LocalSIN, sizeof( LocalSIN ) ) == SOCKET_ERROR )
		{
			m_HasError = true;
			m_Error = GetLastError( );
			(*m_CONSOLEPrint)( "[TCPCLIENT] error (bind) - " + GetErrorString( ) );
			return;
		}
	}

	// connect

	m_SIN.sin_family = AF_INET;
	m_SIN.sin_addr.s_addr = HostAddress;
	m_SIN.sin_port = htons( port );

	if( connect( m_Socket, (struct sockaddr *)&m_SIN, sizeof( m_SIN ) ) == SOCKET_ERROR )
	{
		if( GetLastError( ) != EINPROGRESS && GetLastError( ) != EWOULDBLOCK )
		{
			// connect error

			m_HasError = true;
			m_Error = GetLastError( );
			(*m_CONSOLEPrint)( "[TCPCLIENT] error (connect) - " + GetErrorString( ) );
			return;
		}
	}

	m_Connecting = true;
}

void CTCPClient :: Connect( string localaddress, string address, uint16_t port )
{
	if( m_Socket == INVALID_SOCKET || m_HasError || m_Connecting || m_Connected )
		return;

	// get IP address

	struct hostent *HostInfo;
	uint32_t HostAddress;
	HostInfo = gethostbyname( address.c_str( ) );

	if( !HostInfo )
	{
		m_HasError = true;
		// m_Error = h_error;
		(*m_CONSOLEPrint)( "[TCPCLIENT] error (gethostbyname)" );
		return;
	}

	memcpy( &HostAddress, HostInfo->h_addr, HostInfo->h_length );

	Connect(localaddress, HostAddress, port);
}

bool CTCPClient :: CheckConnect( )
{
	if( m_Socket == INVALID_SOCKET || m_HasError || !m_Connecting )
		return false;

	fd_set fd;
	FD_ZERO( &fd );
	FD_SET( m_Socket, &fd );

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	// check if the socket is connected

#ifdef WIN32
	if( select( 1, NULL, &fd, NULL, &tv ) == SOCKET_ERROR )
#else
	if( select( m_Socket + 1, NULL, &fd, NULL, &tv ) == SOCKET_ERROR )
#endif
	{
		m_HasError = true;
		m_Error = GetLastError( );
		return false;
	}

	if( FD_ISSET( m_Socket, &fd ) )
	{
		m_Connecting = false;
		m_Connected = true;
		return true;
	}

	return false;
}

//
// CTCPServer
//

CTCPServer :: CTCPServer(void (*console_print)(string), bool nIsBlocking ) : CTCPSocket( console_print )
{
	// set the socket to reuse the address in case it hasn't been released yet

	int optval = 1;

#ifdef WIN32
	setsockopt( m_Socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof( int ) );
#else
	setsockopt( m_Socket, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof( int ) );
#endif

	SetBlocking(nIsBlocking);
}

CTCPServer :: ~CTCPServer( )
{

}

bool CTCPServer :: Listen( string address, uint16_t port )
{
	if( m_Socket == INVALID_SOCKET || m_HasError )
		return false;

	m_SIN.sin_family = AF_INET;

	if( !address.empty( ) )
	{
		if( ( m_SIN.sin_addr.s_addr = inet_addr( address.c_str( ) ) ) == INADDR_NONE )
			m_SIN.sin_addr.s_addr = INADDR_ANY;
	}
	else
		m_SIN.sin_addr.s_addr = INADDR_ANY;

	m_SIN.sin_port = htons( port );

	if( bind( m_Socket, (struct sockaddr *)&m_SIN, sizeof( m_SIN ) ) == SOCKET_ERROR )
	{
		m_HasError = true;
		m_Error = GetLastError( );
		(*m_CONSOLEPrint)( "[TCPSERVER] error (bind) - " + GetErrorString( ) );
		return false;
	}

	// listen, queue length 8

	if( listen( m_Socket, 8 ) == SOCKET_ERROR )
	{
		m_HasError = true;
		m_Error = GetLastError( );
		(*m_CONSOLEPrint)( "[TCPSERVER] error (listen) - " + GetErrorString( ) );
		return false;
	}

	return true;
}

CTCPSocket *CTCPServer :: Accept( )
{
	struct sockaddr_in Addr;
	int AddrLen = sizeof( Addr );
	SOCKET NewSocket;

#ifdef WIN32
	if( ( NewSocket = accept( m_Socket, (struct sockaddr *)&Addr, &AddrLen ) ) == INVALID_SOCKET )
#else
	if( ( NewSocket = accept( m_Socket, (struct sockaddr *)&Addr, (socklen_t *)&AddrLen ) ) == INVALID_SOCKET )
#endif
	{
		// accept error, ignore it
	}
	else
	{
		// success! return the new socket

		return new CTCPSocket( m_CONSOLEPrint,NewSocket, Addr );
	}
	return NULL;
}

CTCPSocket *CTCPServer :: Accept( fd_set *fd )
{
	if( m_Socket == INVALID_SOCKET || m_HasError )
		return NULL;

	if( FD_ISSET( m_Socket, fd ) )
	{
		// a connection is waiting, accept it
		return Accept();
	}

	return NULL;
}

