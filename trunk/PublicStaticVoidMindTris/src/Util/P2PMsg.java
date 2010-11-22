package Util;

public class P2PMsg extends Msg {
	public static final byte
		HELLO		= 0x00,
		CHAT_SEND	= 0x01,
		ROUND		= 0x02,
		MESSAGE		= 0x04;

	public P2PMsg(byte type, byte[][] data) {
		super(type, data);
	}
}
