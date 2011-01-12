

#ifndef MTC_SOCKET_H
#define MTC_SOCKET_H

#ifdef WIN32
 #include <winsock2.h>
 #include <errno.h>

#else
 #include <arpa/inet.h>
 #include <errno.h>
 #include <fcntl.h>
 #include <netdb.h>
 #include <netinet/in.h>
 #include <netinet/tcp.h>
 #include <sys/ioctl.h>
 #include <sys/socket.h>
 #include <sys/types.h>
 #include <unistd.h>

 typedef int SOCKET;

 #define INVALID_SOCKET -1
 #define SOCKET_ERROR -1

 #define closesocket close

 extern int GetLastError( );
#endif

#ifndef INADDR_NONE
 #define INADDR_NONE -1
#endif

#ifndef MSG_NOSIGNAL
 #define MSG_NOSIGNAL 0
#endif

#ifdef WIN32
 #define SHUT_RDWR 2
#endif

//
// CSocket
//

class CSocket
{
protected:
	void (*m_CONSOLEPrint)(string);
	SOCKET m_Socket;
	struct sockaddr_in m_SIN;
	bool m_HasError;
	int m_Error;
	bool m_IsBlocking;

public:
	CSocket(  void (*consoleprint)(string) );
	CSocket(  void (*consoleprint)(string), SOCKET nSocket, struct sockaddr_in nSIN );
	~CSocket( );

	void SetBlocking(bool nIsBlocking);
	virtual ByteArray GetPort( );
	virtual uint32_t GetIPInt32();
	virtual ByteArray GetIP( );
	virtual string GetIPString( );
	virtual bool HasError( )						{ return m_HasError; }
	virtual int GetError( )							{ return m_Error; }
	virtual string GetErrorString( );
	virtual void SetFD( fd_set *fd );
	virtual void SetFD( fd_set *fd, fd_set *send_fd, int *nfds );
	virtual void Allocate( int type );
	virtual void Reset( );
};

//
// CTCPSocket
//
class ByteBuffer; 
class CTCPSocket : public CSocket
{
protected:
	bool m_Connected;

private:
	ByteBuffer m_RecvBuffer;
	ByteBuffer m_SendBuffer;
	uint32_t m_LastRecv;
	uint32_t m_LastSend;

public:
	CTCPSocket( void (*console_print)(string));
	CTCPSocket( void (*console_print)(string), SOCKET nSocket, struct sockaddr_in nSIN );
	virtual ~CTCPSocket( );

	virtual void Reset( );
	virtual bool GetConnected( )				{ return m_Connected; }
	virtual ByteBuffer & GetRecvBuffer( );
	virtual ByteBuffer & GetSendBuffer( );
	virtual uint32_t GetLastRecv( )				{ return m_LastRecv; }
	virtual uint32_t GetLastSend( )				{ return m_LastSend; }
	virtual void DoRecv();
	virtual void DoSend();
	virtual void DoRecv( fd_set *fd );
	virtual void DoSend( fd_set *send_fd );
	virtual void Disconnect( );
	virtual void SetNoDelay( bool noDelay );
};

//
// CTCPClient
//

class CTCPClient : public CTCPSocket
{
protected:
	bool m_Connecting;

public:
	CTCPClient(void (*console_print)(string) , bool nIsBlocking  );
	virtual ~CTCPClient( );

	virtual void Reset( );
	virtual void Disconnect( );
	virtual bool GetConnecting( )												{ return m_Connecting; }
	virtual void Connect( string localaddress, uint32_t HostAddress, uint16_t port );
	virtual void Connect( string localaddress, string address, uint16_t port );
	virtual bool CheckConnect( );
};

//
// CTCPServer
//

class CTCPServer : public CTCPSocket
{
public:
	CTCPServer(void (*console_print)(string), bool nIsBlocking );
	virtual ~CTCPServer( );

	virtual bool Listen( string address, uint16_t port );
	virtual unique_ptr<CTCPSocket> Accept( );
	virtual unique_ptr<CTCPSocket> Accept( fd_set *fd );
};

#endif
