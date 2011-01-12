
#include "includes.h"
#include "util.h"
#include "bytearray.h"
#include "bytebuffer.h"
#include "commprotocol.h"
#include "p2pprotocol.h"

const ByteArray & DGMTP2PProtocol::GetProtocolIdentifier() const{
	static ByteArray s_protocolidentifier = ByteArray(DGMTP2P_PROTOCOLIDENTIFIER);
	return s_protocolidentifier;
};

const bool DGMTP2PProtocol::IsBigEndian() const{
	static bool s_isbigendian = true;
	return s_isbigendian;
}

DGMTP2PProtocol::DGMTP2PProtocol(): m_parser(IsBigEndian()){}

///////////////////////
// SEND FUNCTIONS    //
///////////////////////


ByteArray DGMTP2PProtocol::SEND_KEEPALIVE(){
	return ByteArray();
}

ByteArray DGMTP2PProtocol::SEND_CHATSEND(uint64_t sessionid,string message, const unique_ptr <CryptoPP::DSA::Signer> & signer)
{
	MessageBuilder builder(IsBigEndian());

	builder.append_BYTE((byte_t) TYPE_CHATSEND);			// DGMTP2P_CHATSEND

	builder.MarkSignatureStart();
	builder.append_INTEGER64(sessionid);
	builder.append_STRING16(message);
	builder.append_DSASignature(m_rng,*signer);

	return builder.GetResult();
}

ByteArray DGMTP2PProtocol::SEND_CONNECTIONREQUEST( uint32_t lobbyid, uint8_t initiating_peerid, uint8_t listening_peerid, uint64_t initiating_challenge){
	MessageBuilder builder(IsBigEndian());
	
	builder.append_BYTE((byte_t) TYPE_CONNECTIONREQUEST);			
	
	builder.append_INTEGER32(lobbyid);
	builder.append_INTEGER8(initiating_peerid);
	builder.append_INTEGER8(listening_peerid);
	builder.append_INTEGER64(initiating_challenge);

	return builder.GetResult();
}

ByteArray DGMTP2PProtocol::SEND_CONNECTIONACCEPTED( uint32_t lobbyid, uint8_t initiating_peerid, uint8_t listening_peerid, uint64_t initiating_challenge,uint64_t listening_challenge, const unique_ptr <CryptoPP::DSA::Signer> & signer){

	MessageBuilder builder(IsBigEndian());
	
	builder.append_BYTE((byte_t) TYPE_CONNECTIONACCEPTED);			
	builder.MarkSignatureStart();
	builder.append_INTEGER32(lobbyid);
	builder.append_INTEGER8(initiating_peerid);
	builder.append_INTEGER8(listening_peerid);
	builder.append_INTEGER64(initiating_challenge);
	builder.append_INTEGER64(listening_challenge);
	builder.append_DSASignature(m_rng,*signer);
	

	return builder.GetResult();
}
ByteArray DGMTP2PProtocol::SEND_CONNECTIONACKNOWLEDGED( uint32_t lobbyid, uint8_t initiating_peerid, uint8_t listening_peerid, uint64_t initiating_challenge,uint64_t listening_challenge,  const unique_ptr <CryptoPP::DSA::Signer> & signer){
	MessageBuilder builder(IsBigEndian());
	
	builder.append_BYTE((byte_t) TYPE_CONNECTIONACKNOWLEDGED);			
	builder.MarkSignatureStart();
	builder.append_INTEGER32(lobbyid);
	builder.append_INTEGER8(initiating_peerid);
	builder.append_INTEGER8(listening_peerid);
	builder.append_INTEGER64(initiating_challenge);
	builder.append_INTEGER64(listening_challenge);
	builder.append_DSASignature(m_rng,*signer);

	return builder.GetResult();
}


ByteArray DGMTP2PProtocol::SEND_ROUND(uint64_t sessionid, uint32_t round_number, vector<Move> moves, vector<RoundDataHash> hashes, const unique_ptr <CryptoPP::DSA::Signer> & signer){
	MessageBuilder builder(IsBigEndian());

	builder.append_BYTE((byte_t) TYPE_ROUND);		
	builder.MarkSignatureStart();

	builder.append_INTEGER64(sessionid);
	builder.append_INTEGER32(round_number);
	builder.append_INTEGER8(moves.size());
	for(vector<Move>::iterator iter = moves.begin(); iter!=moves.end();iter++){
		builder.append_INTEGER32(iter->GetPieceNumber());
		builder.append_INTEGER8(iter->GetOrientation());
		builder.append_INTEGER8(iter->GetPieceXOffset());
		builder.append_INTEGER8(iter->GetPieceYOffset());
	}

	builder.append_INTEGER8(hashes.size());
	for(vector<RoundDataHash>::iterator iter = hashes.begin(); iter!=hashes.end();iter++){
		builder.append_INTEGER8(iter->GetPeerID());
		builder.append_STRING8(iter->GetHash());
	}
	builder.append_DSASignature(m_rng,*signer);
	return builder.GetResult();
}

///////////////////////
// RECEIVE FUNCTIONS //
///////////////////////
	

DGMTP2PProtocol::RoundData DGMTP2PProtocol::RECEIVE_ROUND( const ByteArray & message, size_t & offset,const unique_ptr<CryptoPP::DSA::Verifier> & verifier){
	size_t begin_signature = offset;

	uint64_t sessionid= m_parser.parse_INTEGER64(message,offset);
	uint32_t round_number= m_parser.parse_INTEGER32(message,offset);

	uint8_t move_size= m_parser.parse_INTEGER8(message,offset);

	vector<Move> moves;
	moves.reserve(move_size);

	for(int i = 1; i<=move_size ;i++)
	{
		uint32_t piecenumber= m_parser.parse_INTEGER32(message,offset);
		uint8_t orientation= m_parser.parse_INTEGER8(message,offset);
		uint8_t x= m_parser.parse_INTEGER8(message,offset);
		uint8_t y= m_parser.parse_INTEGER8(message,offset);
		moves.push_back(move(Move(piecenumber,orientation,x,y)));
	}

	uint8_t hashes_size= m_parser.parse_INTEGER8(message,offset);

	for(int i = 1; i<=hashes_size ;i++)
	{
		m_parser.parse_STRING(20,message,offset);
	}

	bool verified = m_parser.parse_DSASignature(message,offset,m_rng, *verifier,begin_signature);

	return RoundData(sessionid,round_number,vector<Move>(), vector<RoundDataHash>(), verified);
}


DGMTP2PProtocol::ConnectionRequest DGMTP2PProtocol::RECEIVE_CONNECTIONREQUEST( const ByteArray & message, size_t & offset){
	size_t begin_signature = offset;

	uint32_t lobbyid= m_parser.parse_INTEGER32(message,offset);
	uint8_t initiating_peerid= m_parser.parse_INTEGER8(message,offset);
	uint8_t listening_peerid= m_parser.parse_INTEGER8(message,offset);
	uint64_t initiating_challenge= m_parser.parse_INTEGER64(message,offset);

	return ConnectionRequest(lobbyid,initiating_peerid,listening_peerid,initiating_challenge);
}

DGMTP2PProtocol::ConnectionAcceptAck DGMTP2PProtocol::RECEIVE_CONNECTIONACCEPTED( const ByteArray & message, size_t & offset,const unique_ptr<CryptoPP::DSA::Verifier> & verifier){
	size_t begin_signature = offset;

	uint32_t lobbyid= m_parser.parse_INTEGER32(message,offset);
	uint8_t initiating_peerid= m_parser.parse_INTEGER8(message,offset);
	uint8_t listening_peerid= m_parser.parse_INTEGER8(message,offset);
	uint64_t initiating_challenge= m_parser.parse_INTEGER64(message,offset);
	uint64_t listening_challenge= m_parser.parse_INTEGER64(message,offset);
	bool verified = m_parser.parse_DSASignature(message,offset,m_rng, *verifier,begin_signature);
    return DGMTP2PProtocol::ConnectionAcceptAck(verified,lobbyid,initiating_peerid,listening_peerid,initiating_challenge,listening_challenge);
}

DGMTP2PProtocol::ConnectionAcceptAck DGMTP2PProtocol::RECEIVE_CONNECTIONACKNOWLEDGED(const ByteArray & message, size_t & offset,const unique_ptr<CryptoPP::DSA::Verifier> & verifier){
	size_t begin_signature = offset;

	uint32_t lobbyid= m_parser.parse_INTEGER32(message,offset);
	uint8_t initiating_peerid= m_parser.parse_INTEGER8(message,offset);
	uint8_t listening_peerid= m_parser.parse_INTEGER8(message,offset);
	uint64_t initiating_challenge= m_parser.parse_INTEGER64(message,offset);
	uint64_t listening_challenge= m_parser.parse_INTEGER64(message,offset);

	bool verified = m_parser.parse_DSASignature(message,offset,m_rng, *verifier,begin_signature);

    return DGMTP2PProtocol::ConnectionAcceptAck(verified,lobbyid,initiating_peerid,listening_peerid,initiating_challenge,listening_challenge);
}



DGMTP2PProtocol::ChatSend DGMTP2PProtocol::RECEIVE_CHATSEND(const ByteArray & message, size_t & offset,const unique_ptr<CryptoPP::DSA::Verifier> & verifier)
{
	size_t begin_signature = offset;
	uint64_t sessionid = m_parser.parse_INTEGER64(message,offset);
	string clientmessage = m_parser.parse_STRING16(message,offset);
	bool verified = m_parser.parse_DSASignature(message,offset,m_rng, *verifier,begin_signature);
	return ChatSend(verified, sessionid,clientmessage);
}

byte_t DGMTP2PProtocol::GetMessageType(const ByteArray &message, size_t & offset){
	return m_parser.parse_BYTE(message,offset);
}


