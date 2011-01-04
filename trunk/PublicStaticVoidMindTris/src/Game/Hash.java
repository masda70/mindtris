package Game;

import java.io.IOException;

import Encodings.Encodable;
import IO.InData;
import IO.OutData;

public class Hash implements Encodable {

	public Hash(InData in) {
		// TODO Auto-generated constructor stub
	}

	@Override
	public void toBytes(OutData out) throws IOException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public int len() {
		return encodingLen();
	}

	public static int encodingLen() {
		// TODO Auto-generated method stub
		return 0;
	}

}
