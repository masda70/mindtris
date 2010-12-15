package Encodings;

import java.io.IOException;
import java.io.Serializable;
import java.math.BigInteger;
import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.PublicKey;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.RSAPublicKeySpec;

import IO.*;

public class RSAKey implements Serializable, Encodable {
	////// STATIC //////
	public static RSAKey nullKey = new RSAKey ();
	private static final long serialVersionUID = 1L;
	
	////// FIELDS //////
	private PublicKey _k;
	private byte[] _mod;
	private byte[] _exp;
	
	////// CONSTRUCTORS //////
	public RSAKey ( PublicKey k ) {
		_k = k;

		try {
			KeyFactory fact = KeyFactory.getInstance("RSA");
			RSAPublicKeySpec spec = fact.getKeySpec(_k, RSAPublicKeySpec.class);
			
			_mod = spec.getModulus().toByteArray();
			_exp = spec.getPublicExponent().toByteArray();
		} catch ( NoSuchAlgorithmException e ) {
			e.printStackTrace();
		} catch (InvalidKeySpecException e) {
			e.printStackTrace();
		}
	}

	public RSAKey ( InData in ) throws IOException {
		try {
			int modLen = in.readUnsignedShort();
			// TODO DEBUG
			byte[] debugMod = new byte[modLen];
			in.readFully(debugMod);
			_mod = new byte[1+modLen];
			_mod[0] = 0x00;
			System.arraycopy(debugMod, 0, _mod, 1, modLen);
			
			int expLen = in.readUnsignedByte();
			_exp = new byte[expLen];
			in.readFully(_exp);
			
			RSAPublicKeySpec spec = new RSAPublicKeySpec(
					new BigInteger(_mod),
					new BigInteger(_exp));
			
			KeyFactory fact = KeyFactory.getInstance("RSA");
			_k = fact.generatePublic(spec);
		} catch ( InvalidKeySpecException e ) {
			e.printStackTrace();
		} catch (NoSuchAlgorithmException e) {
			e.printStackTrace();
		}
	}

	private RSAKey () {
		_mod = new byte[0];
		_exp = new byte[0];
	}
	
	////// ENCODING //////
	public void toBytes ( OutData out ) throws IOException {
		out.writeShort(_mod.length - 1);
		// TODO DEBUG
		byte[] modDebug = new byte[_mod.length - 1];
		System.arraycopy(_mod, 1, modDebug, 0, _mod.length-1);
		out.write(modDebug);
		out.writeByte(_exp.length);
		out.write(_exp);
	}

	public int len () {
		// TODO DEBUG
		return 2 + _mod.length -1 + 1 + _exp.length;
	}

	////// PUBLIC METHODS //////
	public PublicKey getKey() {
		return _k;
	}
}
