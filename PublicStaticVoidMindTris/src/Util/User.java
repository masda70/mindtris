package Util;

import Encodings.*;

import java.io.DataInput;
import java.io.IOException;
import java.io.Serializable;

import javax.crypto.Cipher;

import IO.*;


public class User implements Serializable, Encodable {
	////// STATIC //////
	private static final long serialVersionUID = 1L;
	
	////// FIELDS //////
	public UString _name,
				   _displayName;
	public AString _email;
	public AString _pwd;
	private transient boolean _isConnected;	// TODO
	private transient Crypted _crypyedPwd;
	private transient int _createdLobbyId = -1;
	
	////// CONSTRUCTORS //////
	public User ( UString name, UString displayName, AString email, AString pwd, Cipher crypter ) throws IOException {
		_name = name;
		_displayName = displayName;
		_email = email;
		_pwd = pwd;
		if( pwd != null ) _crypyedPwd = new Crypted(pwd, crypter);
	}
	
	public User ( InData in, Cipher decrypter ) throws IOException {
		int nameLen = in.readUnsignedByte();
		_name = new UString(in, nameLen);
		
		int displayNameLen = in.readUnsignedByte();
		_displayName = new UString(in, displayNameLen);
		
		int emailLen = in.readUnsignedShort();
		_email = new AString(in, emailLen);
		
		int pwdLen = in.readUnsignedShort();
		_crypyedPwd = new Crypted(in, pwdLen);
		_pwd = _crypyedPwd.decrypt(decrypter);
	}
	
	////// ENCODINGS //////
	public void toBytes ( OutData out ) throws IOException {
		out.writeByte(_name.len());
		out.write(_name);
		out.writeByte(_displayName.len());
		out.write(_displayName);
		out.writeShort(_email.len());
		out.write(_email);
		out.writeShort(_crypyedPwd.len());
		out.write(_crypyedPwd);
	}

	public int len () {
		return 1 + _name.len() + 1 + _displayName.len() + 2 + _email.len() + 2 + _crypyedPwd.len();
	}
	
	////// PUBLIC METHODS //////
	// TODO check validity
	public boolean isNameValid () {
		return _name != null;
	}
	
	public boolean isEmailValid () {
		return _email != null;
	}

	public boolean isPwdValid () {
		return _pwd != null;
	}

	public int getCreatedLobbyId() throws IOException {
		if( _createdLobbyId == -1 ) throw new IOException("no lobby created");
		
		return _createdLobbyId;
	}

	public void setCreatedLobbyId ( int lobbyId ) {
		_createdLobbyId = lobbyId;
	}
}
