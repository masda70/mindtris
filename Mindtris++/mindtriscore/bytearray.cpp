#include "includes.h"
#include "util.h"
#include "bytearray.h"

ByteArray::ByteArray(ByteArray&& other) //move constructor
{
	m_data = move(other.m_data);
}


ByteArray& ByteArray::operator =(ByteArray&& other) // move assignment
{
	m_data = move(other.m_data);
	return *this;
}

ByteArray & ByteArray::operator = (const ByteArray & other){
	m_data = other.m_data;
	return *this;
}

ByteArray::ByteArray(const ByteArray& from)
{
	m_data = from.m_data;
}

void ByteArray::append( const ByteArray &append )
{
	m_data.insert( m_data.end( ), append.m_data.begin( ), append.m_data.end( ) );
}

void ByteArray::append( byte c)
{
	m_data.push_back(c);
}

void ByteArray::append( const byte *a, int size )
{
	m_data.insert(m_data.end(),a,a+size);
}

void ByteArray::append( const string & append, bool terminator  )
{
	m_data.insert(m_data.end(),append.begin(),append.end());
	if( terminator )
		m_data.push_back( 0 );
}

void ByteArray::append( const string & append, int size, bool terminator  )
{
	m_data.insert(m_data.end(),append.begin(),append.begin()+size);
	if( terminator )
		m_data.push_back( 0 );
}

void ByteArray::append( bool b )
{
	m_data.push_back( (byte) b?0x01:0x00 );
}


void ByteArray::append( uint8_t i, bool reverse )
{
	m_data.push_back( (byte) i );
}

void ByteArray::append( uint16_t i, bool reverse )
{
	if(reverse)
	{
		m_data.push_back( (byte)( i >> 8 ) );
		m_data.push_back( (byte)i );
	}else{
		m_data.push_back( (byte)i );
		m_data.push_back( (byte)( i >> 8 ) );
	}
}

void ByteArray::append( uint32_t i, bool reverse )
{
	if(reverse)
	{
		m_data.push_back( (byte)( i >> 24 ) );
		m_data.push_back( (byte)( i >> 16 ) );
		m_data.push_back( (byte)( i >> 8 ) );
		m_data.push_back( (byte)i );
	}else{
		m_data.push_back( (byte)i );
		m_data.push_back( (byte)( i >> 8 ) );
		m_data.push_back( (byte)( i >> 16 ) );
		m_data.push_back( (byte)( i >> 24 ) );
	}
}

void ByteArray::append( uint64_t i, bool reverse )
{
	if(reverse)
	{
		m_data.push_back( (byte)( i >> 56 ) );
		m_data.push_back( (byte)( i >> 48 ) );
		m_data.push_back( (byte)( i >> 40 ) );
		m_data.push_back( (byte)( i >> 32 ) );
		m_data.push_back( (byte)( i >> 24 ) );
		m_data.push_back( (byte)( i >> 16 ) );
		m_data.push_back( (byte)( i >> 8 ) );
		m_data.push_back( (byte) i );
	}else{
		m_data.push_back( (byte)i );
		m_data.push_back( (byte)( i >> 8 ) );
		m_data.push_back( (byte)( i >> 16 ) );
		m_data.push_back( (byte)( i >> 24 ) );
		m_data.push_back( (byte)( i >> 32 ) );
		m_data.push_back( (byte)( i >> 40 ) );
		m_data.push_back( (byte)( i >> 48 ) );
		m_data.push_back( (byte)( i >> 56 ) );
	}
}

ByteArray::ByteArray( uint16_t i, bool reverse )
{
	this->append(i,reverse);
}

ByteArray::ByteArray( uint32_t i, bool reverse )
{
	this->append(i,reverse);
}

ByteArray::ByteArray( uint64_t i, bool reverse )
{
	this->append(i,reverse);
}

bool ByteArray::extract_bool( size_t offset ) const {
	return (m_data.at(offset)==0x00)?false:true;
}
byte_t ByteArray::extract_byte(  size_t offset ) const {
	return m_data.at(offset);
}
string ByteArray::extract_string( size_t offset, size_t length ) const {
	return string(m_data.begin()+offset,m_data.begin()+offset+length);
}
uint8_t ByteArray::extract_uint8( bool reverse, size_t start ) const 
{
	if( m_data.size( ) < start + 1 )
		return 0;

	return (uint8_t)( m_data.at(start) );
}

uint16_t ByteArray::extract_uint16(  bool reverse, size_t start ) const 
{
	if( m_data.size( ) < start + 2 )
		return 0;

	if( reverse )
	{
		return (uint16_t)( m_data.at(start+0) << 8 |  m_data.at(start+1) );
	}else{
		return (uint16_t)( m_data.at(start+1) << 8 |  m_data.at(start+0) );
	}
}

uint32_t ByteArray::extract_uint32(  bool reverse, size_t start ) const 
{
	if( m_data.size( ) < start + 4 )
		return 0;

	if( reverse )
		return (uint32_t)( m_data.at(start+0) << 24 | m_data.at(start+1) << 16 | m_data.at(start+2) << 8 | m_data.at(start+3) );
	else
		return (uint32_t)( m_data.at(start+3) << 24 | m_data.at(start+2) << 16 | m_data.at(start+1) << 8 | m_data.at(start+0) );
}

uint64_t ByteArray::extract_uint64( bool reverse, size_t start ) const 
{
	if( m_data.size( ) < start + 8 )
		return 0;

	if( reverse )
		return (uint64_t)( ((uint64_t) m_data.at(start+0)) << 56 | ((uint64_t) m_data.at(start+1)) << 48 | ((uint64_t) m_data.at(start+2)) << 40 | ((uint64_t) m_data.at(start+3)) << 32 | ((uint64_t) m_data.at(start+4)) << 24 | ((uint64_t) m_data.at(start+5)) << 16 | ((uint64_t) m_data.at(start+6)) << 8 | ((uint64_t) m_data.at(start+7)) );
	else
		return (uint64_t)( ((uint64_t) m_data.at(start+7)) << 56 | ((uint64_t) m_data.at(start+6)) << 48 | ((uint64_t) m_data.at(start+5)) << 40 | ((uint64_t) m_data.at(start+4)) << 32 | ((uint64_t) m_data.at(start+3)) << 24 | ((uint64_t) m_data.at(start+2)) << 16 | ((uint64_t) m_data.at(start+1)) << 8 | ((uint64_t) m_data.at(start+0)) );
}

string ByteArray::to_decstring(  )
{
	if( m_data.empty( ) )
		return string( );

	string result = UTIL_ToString( m_data.at(0) );

	for( vector<byte>::iterator i = m_data.begin( ) + 1; i != m_data.end( ); i++ )
		result += " " + UTIL_ToString( *i );

	return result;
}

string ByteArray::to_hexstring(  )
{
	if( m_data.empty( ) )
		return string( );

	string result = UTIL_ToHexString( m_data.at(0) );

	for( vector<byte>::iterator i = m_data.begin( ) + 1; i != m_data.end( ); i++ )
	{
		if( *i < 16 )
			result += " 0" + UTIL_ToHexString( *i );
		else
			result += " " + UTIL_ToHexString( *i );
	}

	return result;
}


