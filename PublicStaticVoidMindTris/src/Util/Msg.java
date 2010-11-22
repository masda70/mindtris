package Util;

import java.lang.reflect.Array;
import java.util.*;

public class Msg {
	public static final byte
		C_HELLO =		 	0x00,
		CREATE_USER =	 	0x01,
		LOGIN =			 	0x02,
		CREATE_LOBBY =		0x03,
		GET_LOBBY_LIST =	0x04,
		JOIN_LOBBY =		0x05,		
		
		S_HELLO = 			0x00 - 0x80,		
		USR_CREATION = 		0x01 - 0x80,
		LOGIN_REPLY = 		0x02 - 0x80,
		LOBBY_CREATED = 	0x03 - 0x80,
		LOBBY_LIST = 		0x04 - 0x80,
		JOINED_LOBBY = 		0x05 - 0x80,
		UPDATE_CLIENT =		0x08 - 0x80,
		
		UNDEFINED = 		0x7F;
	
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
	
	public Msg ( byte type, byte ans, byte [] ... data ) {
		_type = type;
		_len = 1;
		
		for( byte[] d : data ) _len += d.length;
		_data = new byte[_len];
		_data[0] = ans;
		
		int i=1;
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
