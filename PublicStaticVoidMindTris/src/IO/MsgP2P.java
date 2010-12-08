package IO;

import java.io.IOException;

public class MsgP2P extends Msg {
	////// STATIC //////
	private static final byte[] protocolId = {'D', 'G', 'M', 'T', 'P', '2', 'P'};
	public static final byte
		HELLO		= 0x00,
		CHAT_SEND	= 0x01,
		ROUND		= 0x02,
		MESSAGE		= 0x04;


	////// CONSTRUCTORS //////
	public MsgP2P ( OutData out, int type, int len ) {
		super(out, type, len);
	}

	public MsgP2P ( InData in ) throws IOException {
		super(in);
	}

	////// OVERRIDE //////
	protected byte[] getProtocolId() {
		return protocolId;
	}

}
