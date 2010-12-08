package IO;

import java.io.*;
import java.util.Arrays;

public abstract class Msg {
	////// STATIC //////
	public static final byte[] protocolVersion = {1,2,0,0};
	
	////// FIELDS //////
	public int _type;
	public InData _in;
	public OutData _out;

	////// CONSTRUCTORS //////
	public Msg ( int type, int len ) {
		_out = new OutData(len);
		_type = type;
		wrHeader(type, len);
	}

	public Msg ( OutData out, int type, int len ) {
		_out = out;
		_type = type;
		wrHeader(type, len);
	}
	
	public Msg ( InData in ) throws IOException {
		byte [] ptclId = new byte[getProtocolId().length];

		in.readFully(ptclId);
		if( !Arrays.equals(ptclId, getProtocolId()) )
			throw new IOException("Wrong protocol");
		
		int len = in.readUnsignedShort();
		_type = in.readUnsignedByte();
		
		byte [] data = new byte[len - ptclId.length - 2 - 1];
		in.readFully(data);

		_in = new InData(new ByteArrayInputStream(data));
	}

	////// OVERRIDE //////
	abstract protected byte[] getProtocolId();
	
	////// PROTECTED //////
	protected void wrHeader ( int type, int len ) {
		try {
			_out.write(getProtocolId());
			_out.writeShort(getProtocolId().length + 2 + 1 + len);
			_out.writeByte(type);
		} catch ( IOException e ) {
			e.printStackTrace();
		}
	}
}
