package Util;

import java.io.IOException;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.Signature;

import Encodings.*;
import IO.*;

public class Peer implements Encodable {
	////// FIELDS //////
	public int _port,
			   _id;
	public UString _displayName;
	public Ip _ip;
	public DSAKey _key;
	public Signature _verifier;
	public boolean _isLoaded=false;
	
	private Channel _ch;
	
	////// CONSTRUCTORS //////
	public Peer ( int peerId, UString displayName, Ip ip, int port, DSAKey key ) {
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
		_key = new DSAKey(in);

		try {
			_verifier = Signature.getInstance(SignedMsg.SIGN_SCHEME);
			_verifier.initVerify(_key.getKey());
	//		_verifier.setParameter(SignedMsg.SIGN_SPEC);
		} catch ( InvalidKeyException e ) {
			throw new IOException("invalid peer public key");
		} catch ( NoSuchAlgorithmException e ) {
			e.printStackTrace();
	//	} catch (InvalidAlgorithmParameterException e) {
	//		e.printStackTrace();
		}
		
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
		if( _ch == null ) System.out.println("Peer has no channel !");
		return _ch;
	}
}
