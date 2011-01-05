package IO;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;

public class InData extends DataInputStream {
	public InData ( InputStream in ) {
		super(in);
	}
}
