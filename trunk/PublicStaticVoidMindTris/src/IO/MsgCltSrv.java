package IO;

import java.io.*;

public class MsgCltSrv extends Msg {
	////// STATIC //////
	private static final byte[] protocolId = {'D', 'G', 'M', 'T'};
	public static final int
		C_HELLO				= 0x00,
		CREATE_USER			= 0x01,
		LOGIN				= 0x02,
		CREATE_LOBBY		= 0x03,
		GET_LOBBY_LIST		= 0x04,
		JOIN_LOBBY			= 0x05,
		LEAVE_LOBBY			= 0x06,
		START_GAME			= 0x10,
		LOADED_GAME			= 0x11,
		
		S_HELLO				= 0x80,
		USR_CREATION		= 0x81,
		LOGIN_REPLY			= 0x82,
		LOBBY_CREATED		= 0x83,
		LOBBY_LIST			= 0x84,
		JOINED_LOBBY		= 0x85,
		UPDATE_CLIENT		= 0x88,
		GAME_STARTING		= 0x90,
		LOAD_GAME			= 0x91,
		BEGIN_GAME			= 0x92,
		
		KEEP_ALIVE			= 0x7E,
		UNDEFINED			= 0x7F;
	public static final Msg KEEP_ALIVE_MSG = new KeepAliveMsg();

	////// CONSTRUCTORS //////
	public MsgCltSrv ( OutData out, int type, int len ) {
		super(out, type, len);
	}

	public MsgCltSrv ( InData in ) throws IOException {
		super(in);
	}
	
	public MsgCltSrv ( int type, int l) {
		super(type, l);
	}
	
	protected MsgCltSrv () {}

	////// OVERRIDE //////
	protected byte[] getProtocolId() {
		return protocolId;
	}

	////// PRIVATE CLASSES //////
	private static class KeepAliveMsg extends MsgCltSrv {
		public KeepAliveMsg () {
			_out = new OutData(0);
		}
		
		public void wrHeader ( OutData out ) {
			try {
				out.write(getProtocolId());
				out.writeShort(getProtocolId().length + 2);
			} catch ( IOException e ) {
				e.printStackTrace();
			}
		}
	}
}
