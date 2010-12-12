package Encodings;

import java.io.IOException;
import java.math.BigInteger;
import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.PublicKey;
import java.security.spec.DSAPublicKeySpec;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.RSAPublicKeySpec;

import IO.InData;
import IO.OutData;

public class DSAKey implements Encodable {
	////// FIELDS //////
	private PublicKey _k;
	private byte[] _p, _q, _g, _y;
	
	////// CONSTRUCTORS //////
	public DSAKey ( PublicKey k ) {
		_k = k;

		try {
			KeyFactory fact = KeyFactory.getInstance("DSA");
			DSAPublicKeySpec spec = fact.getKeySpec(_k, DSAPublicKeySpec.class);
			_p = spec.getP().toByteArray();
			_q = spec.getQ().toByteArray();
			_g = spec.getQ().toByteArray();
			_y = spec.getY().toByteArray();
		} catch ( NoSuchAlgorithmException e ) {
			e.printStackTrace();
		} catch (InvalidKeySpecException e) {
			e.printStackTrace();
		}
	}
	
	public DSAKey ( InData in ) throws IOException {
		try {
			int pLen = in.readUnsignedShort();
			_p = new byte[pLen];
			in.readFully(_p);

			int qLen = in.readUnsignedShort();
			_q = new byte[qLen];
			in.readFully(_q);
			
			int gLen = in.readUnsignedShort();
			_g = new byte[gLen];
			in.readFully(_g);
			
			int yLen = in.readUnsignedShort();
			_y = new byte[yLen];
			in.readFully(_y);
			
			DSAPublicKeySpec spec = new DSAPublicKeySpec(
					new BigInteger(_y),
					new BigInteger(_p),
					new BigInteger(_q),
					new BigInteger(_g));
			
			KeyFactory fact = KeyFactory.getInstance("DSA");
			_k = fact.generatePublic(spec);
		} catch ( InvalidKeySpecException e ) {
			e.printStackTrace();
		} catch (NoSuchAlgorithmException e) {
			e.printStackTrace();
		}
	}

	////// ENCODINGS //////
	public void toBytes(OutData out) throws IOException {
		out.writeShort(_p.length);
		out.write(_p);
		out.writeShort(_q.length);
		out.write(_q);
		out.writeShort(_g.length);
		out.write(_g);
		out.writeShort(_y.length);
		out.write(_y);
	}

	public int len() {
		return 2 + _p.length + 2 + _q.length + 2 + _g.length + 2 + _y.length;
	}

	////// PUBLIC METHODS //////
	public PublicKey getKey() {
		return _k;
	}

}
