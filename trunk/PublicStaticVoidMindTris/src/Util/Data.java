package Util;

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

	public byte rdB () {
		return _d[_offset++];
	}
	public int rdS () {
		int out = Channel.bytes2short(_d, _offset);
		_offset += 2;
		return out;
	}
	public void rd ( byte[] buf, int len ) {
		buf = new byte[len];
		System.arraycopy(_d, _offset, buf, 0, len);
		_offset += len;
	}
}
