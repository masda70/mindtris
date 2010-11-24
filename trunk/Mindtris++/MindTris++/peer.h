

#ifndef MT_PEER_H
#define MT_PEER_H



class CTCPClient;
class CTCPServer;
class DGMTPacket;
class DGMTProtocol;
class DGMTP2PProtocol;

class MindTrisClient;

class PeerInfo
{
private:
	uint8_t m_peerid;
	string m_displayname;
	uint32_t m_ipaddress;
	uint16_t m_portnumber;
	RSAPublicKey * m_publickey;
public:
	uint8_t GetPeerID(){ return m_peerid;}
	uint32_t GetIPAddress(){ return m_ipaddress;}
	uint16_t GetPortNumber(){return m_portnumber;}
	string GetDisplayName(){ return m_displayname;}
	RSAPublicKey * GetPublicKey() { return m_publickey;}
	PeerInfo(uint8_t peerid, string displayname, uint32_t ipaddress, uint16_t portnumber, RSAPublicKey * key){
		m_peerid = peerid;
		m_displayname = displayname;
		m_ipaddress = ipaddress;
		m_portnumber = portnumber;
		m_publickey = key;
	}

};

class Peer
{
private:
	
	bool m_trusted;
	bool m_peerinitiatedhandshake;
	bool m_clientinitiatedhandshake;

	uint8_t m_id;
	uint8_t m_peerid; 

	RSAPublicKey * m_publickey;
	string m_displayname;

	MindTrisClient * m_client;
	DGMTP2PProtocol * m_Protocol;
	
	string m_ErrorString;

	CTCPClient *m_ClientSocket; /* If the client initiated the connection, this is not null */
	CTCPSocket *m_Socket;

	queue<DGMTP2PProtocol::Packet *> m_Packets;

	bool m_Error;
	bool m_DeleteMe;

public:

	void Send( BYTEARRAY data );
	bool Update(fd_set * fd);

	bool GetClientInitiatedHandshake() {return m_clientinitiatedhandshake;}
	string GetDisplayName(){ return m_displayname;}
	void SetDisplayName(string s){ m_displayname = s;}

	void SetPublicKey(RSAPublicKey * s){ m_publickey = s;}
	RSAPublicKey * GetPublicKey(){ return m_publickey;}

	uint8_t GetPeerID(){return m_peerid;}
	void SetPeerID(uint8_t id){ m_peerid = id;}

	void PrintMalformedPacket();


	CTCPClient *GetClientSocket(){ return m_ClientSocket;}
	CTCPSocket *GetSocket( ) { return m_Socket; }
	Peer(MindTrisClient * client, DGMTP2PProtocol * protocol, CTCPSocket * nSocket);
    Peer(MindTrisClient * client, DGMTP2PProtocol * protocol, uint8_t peerid, string displayname, uint32_t ipaddress, uint16_t port, RSAPublicKey * publickey);
	~Peer();
};

#endif //MT_PEER_H