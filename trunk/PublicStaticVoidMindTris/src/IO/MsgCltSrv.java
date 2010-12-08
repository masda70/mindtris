package IO;

import java.io.*;

public class MsgCltSrv extends Msg {
	////// STATIC //////
	private static final byte[] protocolId = {'D', 'G', 'M', 'T'};
	public static final int
		C_HELLO =		 	0x00,
		CREATE_USER =	 	0x01,
		LOGIN =			 	0x02,
		CREATE_LOBBY =		0x03,
		GET_LOBBY_LIST =	0x04,
		JOIN_LOBBY =		0x05,		
		
		S_HELLO = 			0x80,
		USR_CREATION = 		0x81,
		LOGIN_REPLY = 		0x82,
		LOBBY_CREATED = 	0x83,
		LOBBY_LIST = 		0x84,
		JOINED_LOBBY = 		0x85,
		UPDATE_CLIENT =		0x88,
		
		UNDEFINED = 		0x7F;

	////// CONSTRUCTORS //////
	public MsgCltSrv ( OutData out, int type, int len ) {
		super(out, type, len);
	}

	public MsgCltSrv ( InData in ) throws IOException {
		super(in);
	}

	////// OVERRIDE //////
	protected byte[] getProtocolId() {
		return protocolId;
	}

}
