
#ifndef MESSAGESTREAMER_H
#define MESSAGESTREAMER_H

class ByteArray;

class MessageStreamer
{
private:
	size_t m_buffersize;
	unique_ptr<byte_t> m_packetbuffer;
	ByteArray m_protocolid;
	bool m_isBigEndian;
	bool PacketExtractData( ByteBuffer& buffer, Packet & packet);
public:
	bool Write( ByteBuffer & buffer, const Message & msg);
	bool Read( ByteBuffer & buffer, unique_ptr<Packet> & incomplete_packet, queue<Message> & packets);
	MessageStreamer(const ByteArray & protocolid, bool isBigEndian, size_t buffersize=1024);
	~MessageStreamer();
};

#endif