package Encodings;

import java.io.IOException;
import java.io.Serializable;
import java.net.InetAddress;
import java.net.UnknownHostException;

import IO.InData;
import IO.OutData;

public class Ip implements Serializable, Encodable {
	////// STATIC //////
	private static final long serialVersionUID = 1L;
	
	////// FIELDS //////
	private byte[] _raw;
	
	////// CONSTRUCTORS //////
	public Ip ( byte[] raw ) {
		_raw = raw;
	}
	
	public Ip ( InData in ) throws IOException {
		_raw = new byte[4];
		in.readFully(_raw);
	}

	////// ENCODINGS //////
	public void toBytes ( OutData out ) throws IOException {
		out.write(_raw);
	}

	public int len() {
		return 4;
	}
	
	////// PUBLIC METHODS //////
	public String toString () {
		try {
			return InetAddress.getByAddress(_raw).getHostAddress();
		} catch (UnknownHostException e) {
			e.printStackTrace();
			return "ERROR";
		}
	}

}
