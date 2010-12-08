package Encodings;

import java.io.IOException;
import java.io.Serializable;
import java.io.UnsupportedEncodingException;
import java.util.Arrays;

import IO.*;

public class UString implements Serializable, Encodable {
	////// STATIC //////
	private static final String ENCODING = "UTF-8";
	private static final long serialVersionUID = 1L;
	
	////// FIELDS //////
	private String _s;
	private byte[] _encoding;
	
	////// CONSTRUCTORS //////
	public UString ( String s ) {
		_s = s;
		try {
			_encoding = _s.getBytes(ENCODING);
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		}
	}

	public UString ( InData in, int len ) throws IOException {
		_encoding = new byte[len];
		in.readFully(_encoding);
		_s = new String(_encoding, ENCODING);
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
		if( o instanceof UString ) {
			return Arrays.equals(_encoding, ((UString) o)._encoding);
		} else {
			return false;
		}
	}
	
	public int hashCode () {
		return _s.hashCode();
	}
}