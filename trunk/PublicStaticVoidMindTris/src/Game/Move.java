package Game;

import java.io.IOException;

import Encodings.Encodable;
import IO.InData;
import IO.OutData;

public class Move implements Encodable {
	////// FIELDS //////
	public final int pieceNb, pieceRotation, pieceX, pieceY;

	////// CONSTRUCTORS //////
	public Move ( int nb, int rot, int x, int y ) {
		pieceNb = nb;
		pieceRotation = rot;
		pieceX = x;
		pieceY = y;
	}
	
	public Move ( InData in ) throws IOException {
		pieceNb = in.readInt();
		pieceRotation = in.readUnsignedByte();
		pieceX = in.readUnsignedByte();
		pieceY = in.readUnsignedByte();
	}

	////// ENCODINGS //////
	public void toBytes(OutData out) throws IOException {
		out.writeInt(pieceNb);
		out.writeByte(pieceRotation);
		out.writeByte(pieceX);
		out.writeByte(pieceY);
	}

	public int len() {
		return encodingLen();
	}
	
	////// STATIC //////
	public static int encodingLen() {
		return 4+1+1+1;
	}

}
