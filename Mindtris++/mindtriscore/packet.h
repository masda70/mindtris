#ifndef PACKET_H
#define PACKET_H

class ByteArray;

class Packet
{
private:
	ByteArray m_Data;
	uint16_t m_Length; // The expected length for m_Data
public:

	Packet( uint16_t nLength)
	{
		m_Length = nLength;
	}

	Packet( ByteArray && data)
	{
		m_Data = forward<ByteArray>(data);
		m_Length = m_Data.size();
	}

	uint16_t GetLength( )	const		{ return m_Length; }
	uint16_t GetEffectiveLength()  const { return m_Data.size(); }
	bool IsIncomplete()		const		{ return m_Data.size()!=m_Length; }
	ByteArray & GetData( )	{ return m_Data; }
	const ByteArray & GetData( )		const	{ return m_Data; }
};

#endif 