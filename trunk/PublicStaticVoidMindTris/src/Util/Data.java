package Util;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;

public class Data {
	private byte[] _d;
	private int _offset;

	public Data ( byte[] d ) {
		_d = d;
		_offset = 0;
	}
	public Data ( byte[] d, int offset ) {
		_d = d;
		_offset = offset;
	}

	public byte[] getBytes () {
		return _d;
	}

	public int getOutLen() {
		return _d.length - _offset;
	}
	
	public byte rdB () {
		return _d[_offset++];
	}
	public boolean rdBool() {
		return ( _d[_offset++] == 0x00 ) ? false : true;
	}
	public int rdS () {
		int out = ByteBuffer.wrap(_d, _offset, 2).getShort();
		_offset += 2;
		return out;
	}
	public int rdI() {
		int out = ByteBuffer.wrap(_d, _offset, 4).getInt();
		_offset += 4;
		return out;
	}
	public long rdL() {
		long out = ByteBuffer.wrap(_d, _offset, 8).getLong();
		_offset += 8;
		return out;
	}
	public void rd ( byte[] buf, int len ) {
		System.arraycopy(_d, _offset, buf, 0, len);
		_offset += len;
	}
	public void rd ( byte[] buf ) {
		rd(buf, getOutLen());
	}
	public String rdStr ( int len ) {
		try {
			String out = new String(_d, _offset, len, Channel.ENCODING);
			_offset += len;
			return out;
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
			return "ERROR";
		}
	}
	public String rdStr () {
		return rdStr(getOutLen());
	}
}
