
#ifndef MTC_INCLUDES_H
#define MTC_INCLUDES_H

// standard integer sizes for 64 bit compatibility

#define __STDC_LIMIT_MACROS

#include <boost/cstdint.hpp>

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
#include <memory>

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

typedef unsigned char byte_t;

// time

uint32_t GetTime( );		// seconds
uint32_t GetTicks( );		// milliseconds

#ifdef WIN32
 #define MILLISLEEP( x ) Sleep( x )
#else
 #define MILLISLEEP( x ) usleep( ( x ) * 1000 )
#endif

class NonCopyable
{
  protected:
    NonCopyable () {}
    ~NonCopyable () {} /// Protected non-virtual destructor
  private: 
    NonCopyable (const NonCopyable &);
    NonCopyable & operator = (const NonCopyable &);
};

//Crypto++



class RSAPublicKey
{
	string Exponent;
	string Modulus;
public:
	RSAPublicKey(string exp, string mod) : Exponent(exp), Modulus(mod){};
	string GetExponent() const{ return Exponent;}
	string GetModulus() const{ return Modulus;}

	RSAPublicKey(){};
	RSAPublicKey(RSAPublicKey&& key) : Exponent(move(key.Exponent)), Modulus(move(key.Modulus)){};
	RSAPublicKey & operator = (RSAPublicKey&& key)
	{
		if(this!=&key)
		{
			Exponent = move(key.Exponent);
			Modulus = move(key.Modulus);
		}
		return *this;
	}

};


class DSAPublicKey
{
	string m_p;
	string m_q;
	string m_g;
	string m_y;
public:
	DSAPublicKey(string p, string q, string g, string y) : m_p(p), m_q(q), m_g(g), m_y(y){};
	string GetP() const{ return m_p;}
	string GetQ() const{ return m_q;}
	string GetG() const{ return m_g;}
	string GetY() const{ return m_y;}

	DSAPublicKey(){};
	DSAPublicKey(DSAPublicKey&& key) : m_p(move(key.m_p)), m_q(move(key.m_q)), m_g(move(key.m_g)), m_y(move(key.m_y)){};
	DSAPublicKey & operator = (DSAPublicKey&& key)
	{
		if(this!=&key)
		{
			m_p=(move(key.m_p)); m_q=(move(key.m_q)); m_g=(move(key.m_g)); m_y =(move(key.m_y));
		}
		return *this;
	}

};


#define CRYPTOPP_DEFAULT_NO_DLL
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include "dll.h"

uint64_t GenerateRandomUINT64(CryptoPP::RandomNumberGenerator &rng);

// network

#undef FD_SETSIZE
#define FD_SETSIZE 512

#endif //MTC_INCLUDES_H
