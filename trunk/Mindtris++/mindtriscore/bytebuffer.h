#ifndef BYTEBUFFER_H
#define BYTEBUFFER_H

class ByteBuffer
{
public:
	typedef byte_t byte;
private:
	size_t m_maxsize;
	size_t m_size;
	byte_t * m_buffer;
	size_t m_begin_pos;
	size_t m_end_pos;
public:
	ByteBuffer(size_t initial_reserve = 1024)
	{
		m_size = 0; m_maxsize = initial_reserve; m_buffer = new byte_t[initial_reserve]; m_begin_pos = 0; m_end_pos=0;
	}

	size_t size() const{ return m_size;}
	size_t peek (byte_t * s, size_t n) const;
	size_t skip( size_t n);
	size_t read( byte_t * s , size_t n);
	size_t write( const byte_t * s , size_t n );
};

#endif