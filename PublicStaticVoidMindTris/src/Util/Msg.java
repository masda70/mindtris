package Util;

public class Msg {
	public static final byte
		C_HELLO = 0x00,			// PROTOCOL VERSION (4 BYTE)
		CREATE_USER = 0x01,
		LOGIN = 0x02,
		CREATE_LOBBY = 0x03,
		GET_LOBBY_LIST = 0x04,
		JOIN_LOBBY = 0x05,		
		SEND_MSG = 0x06,

		// UGLY : pour le test du chat p2p, sert a demander la liste des pairs et a envoyer ses infos...
		TEST_CHAT_LIST_PEERS = 0x07,
		
		S_HELLO = 0x00 - 0x80,	// HELLO ANSWER = 1 BYTE) : 0x00 = SUCCESS, 0x01 = WRONG PROTOCOL VERSION, 0x02 = UNKNOWN ERROR
		 						// MESSAGE FOR CLIENT = UTF-8 STRING, occupe le reste du message): Un message du serveur qui peut lu par un humain. Si HELLO ANSWER= SUCCESS  = 0x00, alors �a sera un message of the day  = MOTD, dans tous les autres cas, ce n'est pas sp�cifi�.
		
		LOGIN_SUCCESS = 0x01 - 0x80,
		LOBBY_CREATED = 0x02 - 0x80,
		
		UNDEFINED = 0x7F;
	
	private byte _type;
	private byte[] _data;
	private short _len;
	
	public Msg ( byte type, byte [] ... data ) {
		_type = type;
		_len = 0;
		
		for( byte[] d : data ) _len += d.length;
		_data = new byte[_len];
		
		int i=0;
		for( byte[] d : data ) {
			System.arraycopy( d, 0, _data, i, d.length);
			i += d.length;
		}
	}
	
	public byte getType () {
		return _type;
	}
	
	public byte [] getData () {
		return _data;
	}
	
	public short getLen () {
		return _len;
	}
	
}
