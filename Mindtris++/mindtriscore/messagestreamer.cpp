#include "includes.h"
#include "bytearray.h"
#include "bytebuffer.h"
#include "packet.h"
#include "commprotocol.h"
#include "messagestreamer.h"

MessageStreamer::~MessageStreamer()
{
}

MessageStreamer::MessageStreamer(const ByteArray & protocolid, bool isBigEndian,size_t buffersize){
	m_buffersize = max(protocolid.size()+2,m_buffersize);
	m_packetbuffer.reset(new byte_t[buffersize]);
	m_protocolid = protocolid;
	m_isBigEndian = isBigEndian;
}

bool MessageStreamer::PacketExtractData( ByteBuffer & buffer, Packet & packet)
{
		size_t missing;
		size_t read;
		do{
			missing = packet.GetLength()-packet.GetData().size();
			read = (size_t) buffer.read(m_packetbuffer.get(),min(missing,m_buffersize));
			packet.GetData().append(m_packetbuffer.get(),read);
			missing = missing - read;
			if(missing != 0 && m_buffersize > read) return false;
		}while(missing>0);
		return true;
}

bool MessageStreamer::Read( ByteBuffer & buffer, unique_ptr<Packet> & incomplete_packet, queue<Message> & messages)
{
	if(incomplete_packet)
	{
		if(!PacketExtractData(buffer,*incomplete_packet)) return true;
		messages.push(move(incomplete_packet->GetData()));
		incomplete_packet.reset(nullptr);
	}
	size_t headerl = m_protocolid.size()+2;
	while( buffer.size() >= headerl )
	{
		buffer.read(m_packetbuffer.get(),headerl);
		for(size_t i=0;i<m_protocolid.size();i++)
		{
			if( *(m_packetbuffer.get()+i)!=m_protocolid.at(i)) return false;
		}
		uint16_t length  = ByteArray(m_packetbuffer.get()+m_protocolid.size(),2).extract_uint16( m_isBigEndian ) - headerl;
		unique_ptr<Packet> packet(new Packet(length));
		if(!PacketExtractData(buffer,*packet)){ incomplete_packet.swap(packet); return true;}
		messages.push(move(packet->GetData()));
	}
	return true;
}


bool MessageStreamer::Write( ByteBuffer & buffer, const Message & message){
	buffer.write( m_protocolid.data(),m_protocolid.size());
	uint16_t length  = message.size()+2+m_protocolid.size();
	buffer.write( ByteArray(length,m_isBigEndian).data(),2);
	buffer.write( message.data(),message.size());
	return true;
}