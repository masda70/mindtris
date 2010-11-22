package Util;

import java.io.Serializable;
import java.io.UnsupportedEncodingException;


public class User implements Serializable {
	private static final long serialVersionUID = 1L;
	public byte[] _displayName,
				  _email,
				  _pwd;
	public String _name;
	
	public User ( String name, byte[] displayName, byte[] email, byte[] pwd ) {
		_name = name;
		_displayName = displayName;
		_email = email;
		_pwd = pwd;
	}
	
	public User ( byte[] encoded ) throws UnsupportedEncodingException {
		int offset=0;

		byte nameLen = encoded[offset++];
		byte [] name = new byte[nameLen];
		System.arraycopy(encoded, offset, name, 0, nameLen);
		offset += nameLen;
		_name = new String(name, Channel.ENCODING);
		
		byte displayNameLen = encoded[offset++];
		_displayName = new byte[displayNameLen];
		System.arraycopy(encoded, offset, _displayName, 0, displayNameLen);
		offset += displayNameLen;
		
		int emailLen = Channel.bytes2short(encoded, offset);
		_email = new byte[emailLen];
		offset += 2;
		System.arraycopy(encoded, offset, _email, 0, emailLen);
		offset += emailLen;
		
		byte pwdLen = encoded[offset++];
		_pwd = new byte[pwdLen];
		System.arraycopy(encoded, offset, _pwd, 0, pwdLen);
	}
	
	public byte[] toBytes () throws UnsupportedEncodingException {
		byte [] name = _name.getBytes(Channel.ENCODING);
		byte nameLen = (byte) name.length,
			 displayNameLen = (byte) _displayName.length,
			 pwdLen = (byte) _pwd.length;
		short emailLen = (short) _email.length;
		byte [] emailLenBuf = Channel.short2bytes(emailLen);
		
		byte [] encoding = new byte [1+nameLen+1+displayNameLen+2+emailLen+1+pwdLen];
		
		int offset = 0;
		
		encoding[offset++] = nameLen;
		System.arraycopy(name, 0, encoding, offset, nameLen);
		offset += nameLen;
		
		encoding[offset++] = displayNameLen;
		System.arraycopy(_displayName, 0, encoding, offset, displayNameLen);
		offset += displayNameLen;
		
		System.arraycopy(emailLenBuf, 0, encoding, offset, 2);
		offset += 2;
		System.arraycopy(_email, 0, encoding, offset, emailLen);
		offset += emailLen;
		
		encoding[offset++] = pwdLen; 
		System.arraycopy(_pwd, 0, encoding, offset, pwdLen);
		
		return encoding;
	}
	
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
}
