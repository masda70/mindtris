#include "includes.h"
#include "util.h"
#include "bytebuffer.h"

size_t ByteBuffer::skip( size_t n)
{
	n = min(m_size,n);
	size_t l = m_maxsize-m_begin_pos;
	if(l<n){
		m_begin_pos = n-l;
		m_size -= n;
	}else{
		m_begin_pos += n;
		if(m_begin_pos >=  m_maxsize) m_begin_pos = 0;
		m_size -= n;
	}
	return n;
}


size_t ByteBuffer::peek ( byte_t * s, size_t n) const
{
	n = min(m_size,n);
	size_t l = m_maxsize-m_begin_pos;
	if(l<n){
		memcpy(s, m_buffer+m_begin_pos,l);
		memcpy(s+l, m_buffer,n-l);
	}else{
		memcpy(s,m_buffer+m_begin_pos,n);
	}
	return n;
}

size_t ByteBuffer::read( byte_t * s , size_t n){
	n = min(m_size,n);
	size_t l = m_maxsize-m_begin_pos;
	if(l<n){
		memcpy(s, m_buffer+m_begin_pos,l);
		memcpy(s+l, m_buffer,n-l);
		m_begin_pos = n-l;
		m_size -= n;
	}else{
		memcpy(s,m_buffer+m_begin_pos,n);
		m_begin_pos += n;
		if(m_begin_pos >=  m_maxsize) m_begin_pos = 0;
		m_size -= n;
	}
	return n;
}

size_t ByteBuffer::write ( const byte_t * s , size_t n ){
	if(n > m_maxsize - m_size){
		size_t new_max = max(m_maxsize*2,m_size+n);
		byte_t * newbuffer = new byte_t[new_max];
		peek(newbuffer,m_size);
		memcpy(newbuffer+m_size, s,n);
		delete[] m_buffer;
		m_maxsize = new_max;
		m_buffer = newbuffer;
		m_begin_pos = 0;
		m_end_pos = m_size + n;
		m_size += n;
		return n;
	}else{
		size_t l = m_maxsize-m_end_pos;
		if(l<n){ 
			// m_begin_pos <= m_end_pos
			memcpy(m_buffer+m_end_pos,s, l);
			memcpy(m_buffer,s+l,n-l);
			m_end_pos = n-l;
			m_size += n;
		}else{
			memcpy(m_buffer+m_end_pos,s,n);
			m_end_pos += n;
			if(m_end_pos >=  m_maxsize) m_end_pos = 0;
			m_size += n;
		}
		return n;
	}
}
