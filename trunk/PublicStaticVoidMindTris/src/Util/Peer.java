package Util;

import java.io.UnsupportedEncodingException;

public class Peer {
	public int _port;
	public byte _id;
	public byte[] _displayName,
				  _ip,
				  _key;
	private Channel _ch;
	
	public Peer ( byte id, byte[] displayName, byte[] ip, int port, byte[] key ) {
		_id = id;
		_displayName = displayName;
		_ip = ip;
		_port = port;
		_key = key;
	}
	
	public Peer ( Data d ) {
		_id = d.rdB();
		byte nameLen = d.rdB();
		_displayName = new byte[nameLen];
		d.rd(_displayName, nameLen);
		_ip = new byte[4];
		d.rd(_ip, 4);
		_port = d.rdS();
		int keyLen = d.rdS();
		_key = new byte[keyLen];
		d.rd(_key, keyLen);
	}

	public byte[] toBytes () {

		byte nameLen = (byte) _displayName.length;
		short keyLen = (short) _key.length;
		byte[] data = new byte[1+1+nameLen+4+2+2+keyLen];
		
		int offset = 0;
		data[offset++] = _id;
		data[offset++] = nameLen;
		System.arraycopy(_displayName, 0, data, offset, nameLen);
		offset += nameLen;
		System.arraycopy(_ip, 0, data, offset, 4);
		offset += 4;
		System.arraycopy(Channel.short2bytes((short)_port), 0, data, offset, 2);
		offset += 2;
		System.arraycopy(Channel.short2bytes(keyLen), 0, data, offset, 2);
		offset += 2;
		System.arraycopy(_key, 0, data, offset, keyLen);
		
		return data;
	}
	
	public String getName () {
		try {
			return new String(_displayName, Channel.ENCODING);
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
			return "ERROR";
		}
	}
	
	public void setCh ( Channel ch ) {
		_ch = ch;
	}
	
	public Channel getCh () {
		if( _ch == null ) System.out.println("User has no channel !");
		return _ch;
	}
}
