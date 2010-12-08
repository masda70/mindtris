package Encodings;

import java.io.IOException;

import IO.OutData;

public interface Encodable {
	public void toBytes ( OutData out ) throws IOException;
	public int len ();
}
