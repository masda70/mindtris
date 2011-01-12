
#include "includes.h"
#include "util.h"
#include "bytearray.h"
#include <sys/stat.h>


string UTIL_ToString( unsigned long i )
{
	string result;
	stringstream SS;
	SS << i;
	SS >> result;
	return result;
}
string UTIL_ToString( unsigned short i )
{
	string result;
	stringstream SS;
	SS << i;
	SS >> result;
	return result;
}

string UTIL_ToString( unsigned int i )
{
	string result;
	stringstream SS;
	SS << i;
	SS >> result;
	return result;
}

string UTIL_ToString( long i )
{
	string result;
	stringstream SS;
	SS << i;
	SS >> result;
	return result;
}

string UTIL_ToString( short i )
{
	string result;
	stringstream SS;
	SS << i;
	SS >> result;
	return result;
}

string UTIL_ToString( int i )
{
	string result;
	stringstream SS;
	SS << i;
	SS >> result;
	return result;
}

string UTIL_ToString( uint64_t i)
{
	string result;
	stringstream SS;
	SS << i;
	SS >> result;
	return result;
}

string UTIL_ToString( float f, int digits )
{
	string result;
	stringstream SS;
	SS << std :: fixed << std :: setprecision( digits ) << f;
	SS >> result;
	return result;
}

string UTIL_ToString( double d, int digits )
{
	string result;
	stringstream SS;
	SS << std :: fixed << std :: setprecision( digits ) << d;
	SS >> result;
	return result;
}

string UTIL_ToHexString( uint32_t i )
{
	string result;
	stringstream SS;
	SS << std :: hex << i;
	SS >> result;
	return result;
}

// todotodo: these UTIL_ToXXX functions don't fail gracefully, they just return garbage (in the uint case usually just -1 casted to an unsigned type it looks like)

uint16_t UTIL_ToUInt16( string &s )
{
	uint16_t result;
	stringstream SS;
	SS << s;
	SS >> result;
	return result;
}

uint32_t UTIL_ToUInt32( string &s )
{
	uint32_t result;
	stringstream SS;
	SS << s;
	SS >> result;
	return result;
}

int16_t UTIL_ToInt16( string &s )
{
	int16_t result;
	stringstream SS;
	SS << s;
	SS >> result;
	return result;
}

int32_t UTIL_ToInt32( string &s )
{
	int32_t result;
	stringstream SS;
	SS << s;
	SS >> result;
	return result;
}

double UTIL_ToDouble( string &s )
{
	double result;
	stringstream SS;
	SS << s;
	SS >> result;
	return result;
}

string UTIL_MSToString( uint32_t ms )
{
	string MinString = UTIL_ToString( ( ms / 1000 ) / 60 );
	string SecString = UTIL_ToString( ( ms / 1000 ) % 60 );

	if( MinString.size( ) == 1 )
		MinString.insert( 0, "0" );

	if( SecString.size( ) == 1 )
		SecString.insert( 0, "0" );

	return MinString + "m" + SecString + "s";
}


string UTIL_AddPathSeperator( string path )
{
	if( path.empty( ) )
		return string( );

#ifdef WIN32
	char Seperator = '\\';
#else
	char Seperator = '/';
#endif

	if( *(path.end( ) - 1) == Seperator )
		return path;
	else
		return path + string( 1, Seperator );
}


bool UTIL_IsLanIP( const ByteArray & ip )
{
	if( ip.size( ) != 4 )
		return false;

	// thanks to LuCasn for this function

	// 127.0.0.1
	if( ip[0] == 127 && ip[1] == 0 && ip[2] == 0 && ip[3] == 1 )
		return true;

	// 10.x.x.x
	if( ip[0] == 10 )
		return true;

	// 172.16.0.0-172.31.255.255
	if( ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31 )
		return true;

	// 192.168.x.x
	if( ip[0] == 192 && ip[1] == 168 )
		return true;

	// RFC 3330 and RFC 3927 automatic address range
	if( ip[0] == 169 && ip[1] == 254 )
		return true;

	return false;
}

bool UTIL_IsLocalIP( const ByteArray & ip, const vector<ByteArray> &localIPs )
{
	if( ip.size( ) != 4 )
		return false;

	for( vector<ByteArray> :: const_iterator i = localIPs.begin( ); i != localIPs.end( ); i++ )
	{
		if( (*i).size( ) != 4 )
			continue;

		if( ip[0] == (*i)[0] && ip[1] == (*i)[1] && ip[2] == (*i)[2] && ip[3] == (*i)[3] )
			return true;
	}

	return false;
}

void UTIL_Replace( string &Text, string Key, string Value )
{
	// don't allow any infinite loops

	if( Value.find( Key ) != string :: npos )
		return;

	string :: size_type KeyStart = Text.find( Key );

	while( KeyStart != string :: npos )
	{
		Text.replace( KeyStart, Key.size( ), Value );
		KeyStart = Text.find( Key );
	}
}

vector<string> UTIL_Tokenize( string s, char delim )
{
	vector<string> Tokens;
	string Token;

	for( string :: iterator i = s.begin( ); i != s.end( ); i++ )
	{
		if( *i == delim )
		{
			if( Token.empty( ) )
				continue;

			Tokens.push_back( Token );
			Token.clear( );
		}
		else
			Token += *i;
	}

	if( !Token.empty( ) )
		Tokens.push_back( Token );

	return Tokens;
}

uint32_t UTIL_Factorial( uint32_t x )
{
	uint32_t Factorial = 1;

	for( uint32_t i = 2; i <= x; i++ )
		Factorial *= i;

	return Factorial;
}
