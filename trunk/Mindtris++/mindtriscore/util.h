
#ifndef MTC_UTIL_H
#define MTC_UTIL_H


class ByteArray;
// conversions
string UTIL_ToString( uint64_t i);
string UTIL_ToString( unsigned long i );
string UTIL_ToString( unsigned short i );
string UTIL_ToString( unsigned int i );
string UTIL_ToString( long i );
string UTIL_ToString( short i );
string UTIL_ToString( int i );
string UTIL_ToString( float f, int digits );
string UTIL_ToString( double d, int digits );
string UTIL_ToHexString( uint32_t i );
uint16_t UTIL_ToUInt16( string &s );
uint32_t UTIL_ToUInt32( string &s );
int16_t UTIL_ToInt16( string &s );
int32_t UTIL_ToInt32( string &s );
double UTIL_ToDouble( string &s );
string UTIL_MSToString( uint32_t ms );

// files

bool UTIL_FileExists( string file );
string UTIL_FileRead( string file, uint32_t start, uint32_t length );
string UTIL_FileRead( string file );
bool UTIL_FileWrite( string file, unsigned char *data, uint32_t length );
string UTIL_FileSafeName( string fileName );
string UTIL_AddPathSeperator( string path );

// other

bool UTIL_IsLanIP( const ByteArray & ip );
bool UTIL_IsLocalIP( const ByteArray & ip, const vector<ByteArray> &localIPs );
void UTIL_Replace( string &Text, string Key, string Value );
vector<string> UTIL_Tokenize( string s, char delim );

// math

uint32_t UTIL_Factorial( uint32_t x );

#define nCr(n, r) (UTIL_Factorial(n) / UTIL_Factorial((n)-(r)) / UTIL_Factorial(r))
#define nPr(n, r) (UTIL_Factorial(n) / UTIL_Factorial((n)-(r)))


#endif