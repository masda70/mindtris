package Encodings;

import java.io.IOException;
import java.math.BigInteger;
import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.PublicKey;
import java.security.spec.DSAPublicKeySpec;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.RSAPublicKeySpec;

import IO.Channel;
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
			_g = spec.getG().toByteArray();
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
			// TODO DEBUG
			byte[] debugP = new byte[pLen];
			in.readFully(debugP);
			_p = new byte[1+pLen];
			_p[0] = 0x00;
			System.arraycopy(debugP, 0, _p, 1, pLen);

			int qLen = in.readUnsignedShort();
			// TODO DEBUG
			byte[] debugQ = new byte[qLen];
			in.readFully(debugQ);
			_q = new byte[1+qLen];
			_q[0] = 0x00;
			System.arraycopy(debugQ, 0, _q, 1, qLen);
			
			int gLen = in.readUnsignedShort();
			// TODO DEBUG
			byte[] debugG = new byte[gLen];
			in.readFully(debugG);
			_g = new byte[1+gLen];
			_g[0] = 0x00;
			System.arraycopy(debugG, 0, _g, 1, gLen);
			
			int yLen = in.readUnsignedShort();
			// TODO DEBUG
			byte[] debugY = new byte[yLen];
			in.readFully(debugY);
			_y = new byte[1+yLen];
			_y[0] = 0x00;
			System.arraycopy(debugY, 0, _y, 1, yLen);
			
			/*
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
			*/
			
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

		// TODO DEBUG
		out.writeShort(_p.length - 1);
		byte[] pDebug = new byte[_p.length - 1];
		System.arraycopy(_p, 1, pDebug, 0, _p.length-1);
		out.write(pDebug);
		
		// TODO DEBUG
		out.writeShort(_q.length - 1);
		byte[] qDebug = new byte[_q.length - 1];
		System.arraycopy(_q, 1, qDebug, 0, _q.length-1);
		out.write(qDebug);
		
		// TODO DEBUG
		out.writeShort(_g.length - 1);
		byte[] gDebug = new byte[_g.length - 1];
		System.arraycopy(_g, 1, gDebug, 0, _g.length-1);
		out.write(gDebug);
		
		// TODO DEBUG
		out.writeShort(_y.length - 1);
		byte[] yDebug = new byte[_y.length - 1];
		System.arraycopy(_y, 1, yDebug, 0, _y.length-1);
		out.write(yDebug);
		
		/*
		out.writeShort(_p.length);
		out.write(_p);
		out.writeShort(_q.length);
		out.write(_q);
		out.writeShort(_g.length);
		out.write(_g);
		out.writeShort(_y.length);
		out.write(_y);
		*/
	}

	public int len() {
		// TODO DEBUG
		return 2 + _p.length -1 + 2 + _q.length -1 + 2 + _g.length -1 + 2 + _y.length -1;
	}

	////// PUBLIC METHODS //////
	public PublicKey getKey() {
		return _k;
	}

}
