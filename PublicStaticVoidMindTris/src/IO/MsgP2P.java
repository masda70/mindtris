package IO;

import java.io.IOException;

public class MsgP2P extends Msg {
	////// STATIC //////
	private static final byte[] protocolId = {'D', 'G', 'M', 'T', 'P', '2', 'P'};
	public static final int
		CONNEXION_REQUEST		= 0x00,
		CONNEXION_ACCEPTED		= 0x01,
		CONNEXION_ACK			= 0x02,
		
		CHAT_SEND				= 0x10,
		
		UNDIFINED				= 0xFF;


	////// CONSTRUCTORS //////
	public MsgP2P ( OutData out, int type, int len ) {
		super(out, type, len);
	}

	public MsgP2P ( InData in ) throws IOException {
		super(in);
	}

	public MsgP2P ( int type, int l) {
		super(type, l);
	}
	
	protected MsgP2P () {}

	////// OVERRIDE //////
	protected byte[] getProtocolId() {
		return protocolId;
	}

}
