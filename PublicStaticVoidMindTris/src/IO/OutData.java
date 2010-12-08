package IO;

import java.io.*;

import Encodings.Encodable;

public class OutData extends DataOutputStream {
	////// CONSTRUCTORS //////
	public OutData ( OutputStream out ) {
		super(out);
	}
	
	public OutData ( int len ) {
		this(new ByteArrayOutputStream(len));
	}
	
	////// PUBLIC METHODS //////
	public void write ( Encodable o ) throws IOException {
		o.toBytes(this);
	}
	
	public byte[] getData() throws IOException {
		if( out instanceof ByteArrayOutputStream )
			return ((ByteArrayOutputStream) out).toByteArray();
		else
			throw new IOException("trying to get unbuffered data ("+out.getClass()+")");
	}
}
