#include "includes.h"
#include "bytearray.h"
#include "commprotocol.h"

ByteArray && MessageBuilder::GetResult(){
	return move(m_message);
}

void MessageBuilder::append_USTRING8( USTRING s){
	if(s.size()>UINT8_MAX) throw range_error( "append_USTRING8: string too long" );
	m_message.append((uint8_t)s.size(),m_isBigEndian);
	m_message.append(s);
}

void MessageBuilder::append_USTRING16( USTRING s){
	if(s.size()>UINT16_MAX) throw range_error( "append_USTRING16: string too long" );
	m_message.append((uint16_t)s.size(),m_isBigEndian);
	m_message.append(s);
}

void MessageBuilder::append_STRING8(STRING s){
	if(s.size()>UINT8_MAX) throw range_error( "append_STRING8: string too long" );
	m_message.append((uint8_t)s.size(),m_isBigEndian);
	m_message.append(s);
}

void MessageBuilder::append_STRING16( STRING s){
	if(s.size()>UINT16_MAX) throw range_error( "append_STRING16: string too long" );
	m_message.append((uint16_t)s.size(),m_isBigEndian);
	m_message.append(s); 
}

void MessageBuilder::append_STRING( STRING s){
	m_message.append(s);
}

void MessageBuilder::append_BYTE(BYTE b){
	m_message.append(b);
}


void MessageBuilder::append_INTEGER8(INTEGER8 i){
	m_message.append((INTEGER8)i,m_isBigEndian);
}

void MessageBuilder::append_INTEGER16(INTEGER16 i){
	m_message.append((INTEGER16)i,m_isBigEndian);
}

void MessageBuilder::append_INTEGER32(INTEGER32 i){
	m_message.append((INTEGER32)i,m_isBigEndian);
}

void MessageBuilder::append_INTEGER64(INTEGER64 i){
	m_message.append((INTEGER64)i,m_isBigEndian);
}

void MessageBuilder::append_BOOLEAN(BOOLEAN b){
	m_message.append(b);
}

void MessageBuilder::append_RSAPublicKey(const RSAPublicKey & key){
	append_STRING16(key.GetModulus());
	append_STRING8(key.GetExponent());
}

void MessageBuilder::append_DSAPublicKey(const DSAPublicKey & key){
	append_STRING16(key.GetP());
	append_STRING16(key.GetQ());
	append_STRING16(key.GetG());
	append_STRING16(key.GetY());
}

void MessageBuilder::MarkSignatureStart(){
	m_signatureoffset = m_message.size();
}

void MessageBuilder::append_DSASignature(CryptoPP::RandomNumberGenerator & rng, const CryptoPP::DSA::Signer & signer){
	string signature;
	CryptoPP::StringSource( string(m_message.extract_string(m_signatureoffset,m_message.size()-m_signatureoffset)), true, 
		new CryptoPP::SignerFilter( rng, signer,
			new CryptoPP:: StringSink( signature )
		) // SignerFilter
	); // StringSource
	append_STRING16(signature);
}




MessageParser::USTRING MessageParser::parse_USTRING8(const ByteArray & message, size_t & offset )  const{
	size_t length  = 1;
	if(!(message.size() >= length + offset)) throw malformed_message();
	uint8_t string_length = message.extract_uint8( m_isBigEndian, offset );
	offset += length;
	if(!(message.size() >= string_length + offset)) throw malformed_message();
	string s = message.extract_string(offset,string_length);
	offset += string_length;
	return s;
}
MessageParser::USTRING MessageParser::parse_USTRING16(const ByteArray & message, size_t & offset ) const{
	size_t length  = 2;
	if(!(message.size() >= length + offset)) throw malformed_message();
	uint16_t string_length = message.extract_uint16( m_isBigEndian, offset );
	offset += length;
	if(!(message.size() >= string_length + offset)) throw malformed_message();
	string s = message.extract_string(offset,string_length);
	offset += string_length;
	return s;
}


MessageParser::STRING MessageParser::parse_STRING(uint8_t size, const ByteArray & message, size_t & offset ) const{
	if(!(message.size() >= size + offset)) throw malformed_message();
	string s = message.extract_string(offset,size);
	offset += size;
	return s;
}

MessageParser::STRING MessageParser::parse_STRING8(const ByteArray & message, size_t & offset ) const{
	size_t length  = 1;
	if(!(message.size() >= length + offset)) throw malformed_message();
	uint8_t string_length = message.extract_uint8( m_isBigEndian, offset );
	offset += length;
	if(!(message.size() >= string_length + offset)) throw malformed_message();
	string s = message.extract_string(offset,string_length);
	offset += string_length;
	return s;
}
MessageParser::STRING MessageParser::parse_STRING16(const ByteArray & message, size_t & offset ) const{
	size_t length  = 2;
	if(!(message.size() >= length + offset)) throw malformed_message();
	uint16_t string_length = message.extract_uint16( m_isBigEndian, offset );
	offset += length;
	if(!(message.size() >= string_length + offset)) throw malformed_message();
	string s = message.extract_string(offset,string_length);
	offset += string_length;
	return s;
}

MessageParser::BYTE MessageParser::parse_BYTE(const ByteArray & message, size_t & offset) const{
	size_t length  = 1;
	if(!(message.size() >= length + offset)) throw malformed_message();
	byte_t b = message.extract_byte( offset );
	offset += length;
	return b;
}

MessageParser::INTEGER8 MessageParser::parse_INTEGER8(const ByteArray & message, size_t & offset) const{
	size_t length  = 1;
	if(!(message.size() >= length + offset)) throw malformed_message();
	uint8_t i = message.extract_uint8(m_isBigEndian,  offset );
	offset += length;
	return i;
}
MessageParser::INTEGER16 MessageParser::parse_INTEGER16(const ByteArray & message, size_t & offset) const{
	size_t length  = 2;
	if(!(message.size() >= length + offset)) throw malformed_message();
	uint16_t i = message.extract_uint16(m_isBigEndian, offset );
	offset += length;
	return i;
}

MessageParser::INTEGER32 MessageParser::parse_INTEGER32(const ByteArray & message, size_t & offset) const{
	size_t length  = 4;
	if(!(message.size() >= length + offset)) throw malformed_message();
	uint32_t i = message.extract_uint32(m_isBigEndian,  offset );
	offset += length;
	return i;
}

MessageParser::INTEGER64 MessageParser::parse_INTEGER64(const ByteArray & message, size_t & offset) const{
	size_t length  = 8;
	if(!(message.size() >= length + offset)) throw malformed_message();
	uint64_t i = message.extract_uint64(m_isBigEndian,  offset );
	offset += length;
	return i;
}

MessageParser::BOOLEAN MessageParser::parse_BOOLEAN(const ByteArray & message, size_t & offset) const{
	size_t length  = 1;
	if(!(message.size() >= length + offset)) throw malformed_message();
	bool b = message.extract_bool( offset );
	offset += length;
	return b;
}

RSAPublicKey MessageParser::parse_RSAPublicKey(const ByteArray & message, size_t & offset) const{
	string mod = MessageParser::parse_STRING16(message,offset);
	string exp = MessageParser::parse_STRING8(message,offset);
	return RSAPublicKey(move(exp),move(mod));
}


DSAPublicKey MessageParser::parse_DSAPublicKey(const ByteArray & message, size_t & offset) const{
	string p = MessageParser::parse_STRING16(message,offset);
	string q = MessageParser::parse_STRING16(message,offset);
	string g = MessageParser::parse_STRING16(message,offset);
	string y = MessageParser::parse_STRING16(message,offset);
	return DSAPublicKey(move(p),move(q),move(g),move(y));
}

bool MessageParser::parse_DSASignature(const ByteArray & message, size_t & offset, CryptoPP::RandomNumberGenerator & rng, const CryptoPP::DSA::Verifier & verifier, size_t signed_message_start){

	size_t signed_message_length = offset - signed_message_start;
	string signature = MessageParser::parse_STRING16(message,offset);
	string signed_message = message.extract_string(signed_message_start, signed_message_length);
	CryptoPP::SignatureVerificationFilter svf(
    verifier,NULL, CryptoPP::SignatureVerificationFilter::SIGNATURE_AT_END
	);	
	CryptoPP::StringSource( signed_message+signature, true,
		new CryptoPP::Redirector( svf )
	); // StringSource

	return svf.GetLastResult();
}
