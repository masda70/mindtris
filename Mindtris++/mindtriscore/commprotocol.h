#ifndef COMMPROTOCOL_H
#define COMMPROTOCOL_H


class MessageParser
{
private:
	bool m_isBigEndian;
	size_t m_signatureoffset;
public:

	bool parse_DSASignature(const ByteArray & message, size_t & offset, CryptoPP::RandomNumberGenerator & rng, const CryptoPP::DSA::Verifier & verifier, size_t signed_message_start);

	MessageParser(bool isBigEndian){ m_isBigEndian = isBigEndian;}
	class malformed_message{};

	typedef const string USTRING;
	typedef const string STRING;
	typedef uint8_t INTEGER8;
	typedef uint16_t INTEGER16;
	typedef uint32_t INTEGER32;
	typedef uint64_t INTEGER64;
	typedef bool BOOLEAN;
	typedef byte_t BYTE;

	USTRING parse_USTRING8(const ByteArray & message, size_t & offset) const;
	USTRING parse_USTRING16(const ByteArray & message, size_t & offset) const;
	STRING  parse_STRING(uint8_t size, const ByteArray & message, size_t & offset) const;

	STRING  parse_STRING8(const ByteArray & message, size_t & offset) const;
	STRING parse_STRING16(const ByteArray & message, size_t & offset ) const;

	BYTE parse_BYTE(const ByteArray & message, size_t & offset) const;
	INTEGER8 parse_INTEGER8(const ByteArray & message, size_t & offset) const;
	INTEGER16 parse_INTEGER16(const ByteArray & message, size_t & offset) const;
	INTEGER32 parse_INTEGER32(const ByteArray & message, size_t & offset) const;
	INTEGER64 parse_INTEGER64(const ByteArray & message, size_t & offset) const;
	BOOLEAN parse_BOOLEAN(const ByteArray & message, size_t & offset) const;

	RSAPublicKey parse_RSAPublicKey(const ByteArray & message, size_t & offset) const;
	DSAPublicKey parse_DSAPublicKey(const ByteArray & message, size_t & offset) const;

};

class MessageBuilder
{
private:
	ByteArray m_message;
	bool m_isBigEndian;
	size_t m_signatureoffset;

public:
	exception invalid_size;
	MessageBuilder(bool isBigEndian){ m_signatureoffset = 0; m_isBigEndian = isBigEndian;}

	typedef const string & USTRING;
	typedef const string & STRING;
	typedef uint8_t INTEGER8;
	typedef uint16_t INTEGER16;
	typedef uint32_t INTEGER32;
	typedef uint64_t INTEGER64;
	typedef bool BOOLEAN;
	typedef byte_t BYTE;

	void MarkSignatureStart();
	void append_DSASignature(CryptoPP::RandomNumberGenerator & rng, const CryptoPP::DSA::Signer & signer);

	void append_USTRING8( USTRING s);
	void append_USTRING16( USTRING s);
	void append_STRING8( STRING s);
	void append_STRING16( STRING s);
	void append_STRING( STRING s);

	void append_BYTE(BYTE b);
	void append_INTEGER8(INTEGER8 i);
	void append_INTEGER16(INTEGER16 i);
	void append_INTEGER32(INTEGER32 i);
	void append_INTEGER64(INTEGER64 i);
	void append_BOOLEAN(BOOLEAN b);

	void append_RSAPublicKey(const RSAPublicKey & key);
	void append_DSAPublicKey(const DSAPublicKey & key);

	ByteArray && GetResult();

};

typedef ByteArray Message;


#endif