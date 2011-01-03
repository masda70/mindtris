package IO;

import java.io.*;
import java.security.SignatureException;
import java.util.Arrays;

public abstract class Msg {
	////// STATIC //////
	public static final byte[] protocolVersion = {1,2,0,3};
	
	////// FIELDS //////
	public int _type;
	public int _length;
	public InData _in;
	public OutData _out;

	////// CONSTRUCTORS //////
	public Msg ( int type, int len ) {
		_out = new OutData(len);
		_type = type;
		_length = len;
	}

	public Msg ( OutData out, int type, int len ) {
		_out = out;
		_type = type;
		_length = len;
	}
	
	public Msg ( InData in ) throws IOException {
		byte [] ptclId = new byte[getProtocolId().length];

		in.readFully(ptclId);
		if( !Arrays.equals(ptclId, getProtocolId()) )
			throw new IOException("Wrong protocol");
		
		int len = in.readUnsignedShort();
		if( len > ptclId.length + 2 ) {
			_type = in.readUnsignedByte();
			
			byte [] data = new byte[len - ptclId.length - 2 - 1];
			in.readFully(data);
	
			_in = new InData(new ByteArrayInputStream(data));
		} else {
			_type = MsgCltSrv.KEEP_ALIVE;
		}
	}
	
	protected Msg () {}

	////// OVERRIDE //////
	abstract protected byte[] getProtocolId();
	
	////// PUBLIC METHODS //////
	public void wrHeader ( OutData out ) {
		try {
			out.write(getProtocolId());
			out.writeShort(getProtocolId().length + 2 + 1 + _length);
			out.writeByte(_type);
		} catch ( IOException e ) {
			e.printStackTrace();
		}
	}
	
	public void end() throws IOException {
	}
}
