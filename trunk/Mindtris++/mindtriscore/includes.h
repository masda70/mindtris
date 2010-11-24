
#ifndef MTC_INCLUDES_H
#define MTC_INCLUDES_H

// standard integer sizes for 64 bit compatibility

#ifdef WIN32
 #include "ms_stdint.h"
#else
 #include <stdint.h>
#endif

// STL

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

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

using namespace std;

typedef vector<unsigned char> BYTEARRAY;

// time

uint32_t GetTime( );		// seconds
uint32_t GetTicks( );		// milliseconds

#ifdef WIN32
 #define MILLISLEEP( x ) Sleep( x )
#else
 #define MILLISLEEP( x ) usleep( ( x ) * 1000 )
#endif



//Crypto++

struct RSAPublicKey
{
	string Exponent;
	string Modulus;
};

#define CRYPTOPP_DEFAULT_NO_DLL
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include "dll.h"

// network

#undef FD_SETSIZE
#define FD_SETSIZE 512

#endif //MTC_INCLUDES_H
