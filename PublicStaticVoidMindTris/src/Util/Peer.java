package Util;

import java.io.IOException;

import Encodings.*;
import IO.*;

public class Peer implements Encodable {
	////// FIELDS //////
	public int _port,
			   _id;
	public UString _displayName;
	public Ip _ip;
	public RSAKey _key;
	
	private Channel _ch;
	
	////// CONSTRUCTORS //////
	public Peer ( int peerId, UString displayName, Ip ip, int port, RSAKey key ) {
		_id = peerId;
		_displayName = displayName;
		_ip = ip;
		_port = port;
		_key = key;
	}
	
	public Peer ( InData in ) throws IOException {
		_id = in.readUnsignedByte();
		int nameLen = in.readUnsignedByte();
		_displayName = new UString(in, nameLen);
		_ip = new Ip(in);
		_port = in.readUnsignedShort();
		_key = new RSAKey(in);
	}

	////// ENCODING //////
	public void toBytes ( OutData out ) throws IOException {
		out.writeByte(_id);
		out.writeByte(_displayName.len());
		out.write(_displayName);
		out.write(_ip);
		out.writeShort(_port);
		out.write(_key);
	}
	
	public int len () {
		return 1+1+_displayName.len()+4+2+_key.len();
	}
	
	////// PUBLIC METHODS //////
	public void setCh ( Channel ch ) {
		_ch = ch;
	}
	
	public Channel getCh () {
		if( _ch == null ) System.out.println("User has no channel !");
		return _ch;
	}
}
