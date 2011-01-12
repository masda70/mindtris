#ifndef DGMTP2PPROTOCOL_H
#define DGMTP2PPROTOCOL_H

#define DGMTP2P_PROTOCOLIDENTIFIER "DGMTP2P"


class ByteArray;
class MessageParser;
class DGMTP2PProtocol: NonCopyable
{
private:

	MessageParser m_parser;
	CryptoPP::AutoSeededRandomPool m_rng;
public:
	const ByteArray & GetProtocolIdentifier() const;
	const bool IsBigEndian() const ;
	DGMTP2PProtocol();

	enum Protocol {
		TYPE_KEEPALIVE				= 0xFF,
		TYPE_CONNECTIONREQUEST		= 0x00,	
		TYPE_CONNECTIONACCEPTED		= 0x01,	
		TYPE_CONNECTIONACKNOWLEDGED	= 0x02,	
		TYPE_CHATSEND				= 0x10,
		TYPE_ROUND					= 0x11,
	};

	class Err
	{
		public: Err(){};
	};

	class ConnectionRequest
	{
	private:
		uint32_t m_lobbyid;
		uint8_t m_initiating_peerid;
		uint8_t m_listening_peerid;
		uint64_t m_initiating_challenge;		
	public:
		uint32_t GetLobbyId() const{return m_lobbyid;}
		uint8_t GetInitiatingPeerID() const{return m_initiating_peerid;}
		uint8_t GetListeningPeerID() const{return m_listening_peerid;}
		uint64_t GetInitiatingChallenge() const {return m_initiating_challenge;}
		ConnectionRequest( int32_t lobbyid,uint8_t initiating_peerid, uint8_t listening_peerid, uint64_t initiating_challenge):
		m_lobbyid(lobbyid), m_initiating_peerid(initiating_peerid), m_listening_peerid(listening_peerid), m_initiating_challenge(initiating_challenge){
		}
	};

	class ConnectionAcceptAck
	{
	private:
		bool m_verified;
		uint32_t m_lobbyid;
		uint8_t m_initiating_peerid;
		uint8_t m_listening_peerid;
		uint64_t m_initiating_challenge;	
		uint64_t m_listening_challenge;
	public:
		uint32_t GetLobbyId() const{return m_lobbyid;}
		uint8_t GetInitiatingPeerID() const{return m_initiating_peerid;}
		uint8_t GetListeningPeerID() const{return m_listening_peerid;}
		uint64_t GetInitiatingChallenge() const {return m_initiating_challenge;}
		uint64_t GetListeningChallenge() const {return m_listening_challenge;}
		bool GetVerified() const { return m_verified;}
		ConnectionAcceptAck(bool verified,  int32_t lobbyid,uint8_t initiating_peerid, uint8_t listening_peerid, uint64_t initiating_challenge,  uint64_t listening_challenge):
		m_verified(verified), m_lobbyid(lobbyid), m_initiating_peerid(initiating_peerid), m_listening_peerid(listening_peerid), m_initiating_challenge(initiating_challenge), m_listening_challenge(listening_challenge){
		}
	};
	
	class ChatSend
	{
	private:
		bool m_verified;
		uint64_t m_sessionid;
		string m_message;
	public:
		uint64_t GetSessionID() const { return m_sessionid;}
		string GetMessage() const {return m_message;}
		ChatSend(bool verified, const uint64_t sessionid, const string & message): m_verified(verified), m_message(message), m_sessionid(sessionid){}
	};


	class Move{
	private:
		uint32_t m_piecenumber;
		uint8_t m_orientation;
		uint8_t m_piece_xoffset;
		uint8_t m_piece_yoffset;
	public:
		uint32_t GetPieceNumber() const {return m_piecenumber;}
		uint8_t GetOrientation() const {return m_orientation;}
		uint8_t GetPieceXOffset() const {return m_piece_xoffset;}
		uint8_t GetPieceYOffset() const {return m_piece_yoffset;}
		Move(uint32_t piecenumber,uint8_t orientation,uint8_t x, uint8_t y): m_piecenumber(piecenumber), m_orientation(orientation), m_piece_xoffset(x), m_piece_yoffset(y){}
	};
	class RoundDataHash{
	private:
		uint8_t m_peer_id;
		string m_hash;
	public:
		uint8_t GetPeerID() const {return m_peer_id;}
		string GetHash() const { return m_hash;}
		RoundDataHash(uint8_t peerid, string hash): m_peer_id(peerid), m_hash(hash){}
	};

	class RoundData{
	private:
		uint64_t m_sessionid;
		uint32_t m_roundnumber;
		vector<Move> m_moves;
		vector<RoundDataHash> m_hashes;
		bool m_verified;
	public:
		uint64_t GetSessionID() const { return m_sessionid;}
		uint32_t GetRoundNumber() const {return m_roundnumber;}
		vector<Move> GetMoves() const {return m_moves;}
		vector<RoundDataHash> GetHashes() const {return  m_hashes;}
		bool GetVerified() const {return  m_verified;}
		RoundData(uint64_t sessionid,uint32_t roundnumber, vector<Move> moves, vector<RoundDataHash> hashes, bool verified): m_sessionid(sessionid), m_roundnumber(roundnumber), m_moves(moves), m_hashes(hashes), m_verified(verified){}
	};


	//SEND FUNCTIONS
	ByteArray SEND_KEEPALIVE();

	ByteArray SEND_CONNECTIONREQUEST( uint32_t lobbyid, uint8_t initiating_peerid, uint8_t listening_peerid, uint64_t initiating_challenge);
	ByteArray SEND_CONNECTIONACCEPTED( uint32_t lobbyid, uint8_t initiating_peerid, uint8_t listening_peerid, uint64_t initiating_challenge,uint64_t listening_challenge,  const unique_ptr <CryptoPP::DSA::Signer> & signer);
	ByteArray SEND_CONNECTIONACKNOWLEDGED( uint32_t lobbyid, uint8_t initiating_peerid, uint8_t listening_peerid, uint64_t initiating_challenge,uint64_t listening_challenge,  const unique_ptr <CryptoPP::DSA::Signer> & signer);

	ByteArray SEND_CHATSEND(uint64_t sessionid, string message, const unique_ptr<CryptoPP::DSA::Signer> & signer );

	ByteArray SEND_ROUND(uint64_t sessionid, uint32_t round_number, vector<Move> moves, vector<RoundDataHash> hashes, const unique_ptr<CryptoPP::DSA::Signer> & signer);

	//RECEIVE FUNCTIONS
	ConnectionRequest RECEIVE_CONNECTIONREQUEST(const ByteArray & message, size_t & offset);
	ConnectionAcceptAck RECEIVE_CONNECTIONACCEPTED(const ByteArray & message, size_t & offset, const unique_ptr<CryptoPP::DSA::Verifier> & verifier);
	ConnectionAcceptAck RECEIVE_CONNECTIONACKNOWLEDGED(const ByteArray & message, size_t & offset, const unique_ptr<CryptoPP::DSA::Verifier> & verifier);

	RoundData RECEIVE_ROUND(const ByteArray & message, size_t & offset, const unique_ptr<CryptoPP::DSA::Verifier> & verifier);


	ChatSend RECEIVE_CHATSEND(const ByteArray & message, size_t & offset, const unique_ptr<CryptoPP::DSA::Verifier> & verifier);
	
	byte_t GetMessageType(const ByteArray &message, size_t & offset);

	//Other functions



};


#endif