package Encodings;

import java.io.IOException;
import java.io.Serializable;
import java.util.Arrays;

import IO.*;

public class AString implements Serializable, Encodable {
	////// FIELDS //////
	private String _s;
	private byte[] _encoding;
	private static final long serialVersionUID = 1L;
	
	////// CONSTRUCTORS //////
	public AString ( String s ) {
		_s = s;
		_encoding = _s.getBytes();
	}
	
	public AString ( byte[] encoding ) {
		_encoding = encoding;
		_s = new String(_encoding);
	}

	public AString ( InData in, int len ) throws IOException {
		_encoding = new byte[len];
		in.readFully(_encoding);
		_s = new String(_encoding);
	}
	
	public AString ( char[] pwd ) {
		this( new String(pwd) );
	}

	////// ENCODING //////
	public void toBytes ( OutData out ) throws IOException {
		out.write(_encoding);
	}

	public int len () {
		return _encoding.length;
	}
	
	////// PUBLIC METHODS //////
	public String v () {
		return _s;
	}

	public boolean equals ( Object o ) {
		if( o instanceof AString ) {
			return Arrays.equals(_encoding, ((AString) o)._encoding);
		} else {
			return false;
		}
	}

	// TODO
	public boolean isValid () {
		return true;
	}
}
