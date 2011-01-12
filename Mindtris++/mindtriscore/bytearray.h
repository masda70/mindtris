#ifndef BYTEARRAY_H
#define BYTEARRAY_H

class ByteArray
{
public:
	typedef byte_t byte;
private:

	vector<byte> m_data;
public:
	ByteArray(const ByteArray& from);
	ByteArray() {};
	ByteArray( const string & s ){m_data.insert(m_data.end(),s.begin(), s.end());};
	ByteArray( const byte *a, int size ){ m_data.insert(m_data.end(),a,a+size); };
	ByteArray( byte c ){ m_data.push_back(c);}
	ByteArray(ByteArray&& other); //move constructor
	ByteArray& operator =(ByteArray&& other); // move assignment


	ByteArray( uint16_t i, bool reverse ) ;
	ByteArray( uint32_t i, bool reverse );
	ByteArray( uint64_t i, bool reverse );

	bool extract_bool ( size_t offset ) const;
	byte extract_byte(  size_t offset ) const;
	string extract_string ( size_t offset, size_t length) const;
	uint8_t extract_uint8 (  bool reverse, size_t start = 0 ) const;
	uint16_t extract_uint16 (  bool reverse, size_t start = 0 ) const;
	uint32_t extract_uint32 ( bool reverse, size_t start = 0 ) const;
	uint64_t extract_uint64 ( bool reverse, size_t start = 0 ) const;

	size_t size() const {return m_data.size();}
	void reserve(size_t capacity){ m_data.reserve(capacity);}
	const byte * data() const{ return m_data.data();}

	byte at(size_t i) const {return m_data.at(i);}
	byte assign(size_t i, byte b) { m_data.assign(i,b);}
	byte operator [](size_t i) const { return m_data.operator[](i);}
	byte& operator [](size_t i) { return m_data.operator[](i);}
	ByteArray & operator = (const ByteArray & other);

	string to_decstring( );
	string to_hexstring( );

	void append( bool b);
	void append( const ByteArray & append );
	void append( byte c );
	void append( const byte *a, int size );
	void append( const string & append, bool terminator = false);
	void append( const string & append, int size, bool terminator = false);

	void append( uint8_t i, bool reverse );
	void append( uint16_t i, bool reverse );
	void append( uint32_t i, bool reverse );
	void append( uint64_t i, bool reverse );


};

#endif //BYTEARRAY_H