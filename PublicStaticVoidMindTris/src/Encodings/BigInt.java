package Encodings;

import java.io.IOException;
import java.math.BigInteger;

import IO.Channel;
import IO.InData;
import IO.OutData;

public class BigInt implements Encodable {
	////// FIELDS //////
	private byte[] _encoding;
	private BigInteger _i;
	
	////// CONSTRUCTORS //////
	public BigInt ( InData in, int len ) throws IOException {
		_encoding = new byte[len];
		in.readFully(_encoding);
		
	System.out.print("bigint debug (len "+_encoding.length+") : ");
	Channel.debug(_encoding);
		
		// to two's complement
		int i = 0;
		while (i < len && _encoding[i] == 0) i++;
		
		int extraByte = 1; //(_encoding[i] & 0x80) == 1 ? 1 : 0;
		byte[] r = new byte[len-i+extraByte];
		
		System.arraycopy(_encoding, i, r, extraByte, len-i);
		
	Channel.debug(_encoding);
		
		_i = new BigInteger(r);
	}
	
	public BigInt ( BigInteger i ) {
		_i = i;
		
		_encoding = i.toByteArray();
		
		if (_encoding[0] == 0) {
		    byte[] tmp = new byte[_encoding.length - 1];
		    System.arraycopy(_encoding, 1, tmp, 0, tmp.length);
		    _encoding = tmp;
		}
	}

	////// PUBLIC METHODS ///////
	public BigInteger toBigInteger () {
		return _i;
	}
	
	////// ENCODINGS //////
	public void toBytes(OutData out) throws IOException {
		out.write(_encoding);
	}

	public int len() {
		return _encoding.length;
	}

}
