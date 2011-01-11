package Encodings;

import java.io.IOException;
import java.io.Serializable;
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
	private BigInt _mod;
	private BigInt _exp;
	
	////// CONSTRUCTORS //////
	public RSAKey ( PublicKey k ) {
		_k = k;

		try {
			KeyFactory fact = KeyFactory.getInstance("RSA");
			RSAPublicKeySpec spec = fact.getKeySpec(_k, RSAPublicKeySpec.class);
			
			_mod = new BigInt(spec.getModulus());
			_exp = new BigInt(spec.getPublicExponent());
		} catch ( NoSuchAlgorithmException e ) {
			e.printStackTrace();
		} catch (InvalidKeySpecException e) {
			e.printStackTrace();
		}
	}

	public RSAKey ( InData in ) throws IOException {
		try {
			int modLen = in.readUnsignedShort();
			_mod = new BigInt(in, modLen);
			
			int expLen = in.readUnsignedByte();
			_exp = new BigInt(in, expLen);
			
			RSAPublicKeySpec spec = new RSAPublicKeySpec(
					_mod.toBigInteger(),
					_exp.toBigInteger());
			
			KeyFactory fact = KeyFactory.getInstance("RSA");
			_k = fact.generatePublic(spec);
		} catch ( InvalidKeySpecException e ) {
			e.printStackTrace();
		} catch (NoSuchAlgorithmException e) {
			e.printStackTrace();
		}
	}

	private RSAKey () {
		_mod = null;
		_exp = null;
	}
	
	////// ENCODING //////
	public void toBytes ( OutData out ) throws IOException {
		out.writeShort(_mod.len());
		out.write(_mod);
		out.writeByte(_exp.len());
		out.write(_exp);
	}

	public int len () {
		return 2+_mod.len() + 1+_exp.len();
	}

	////// PUBLIC METHODS //////
	public PublicKey getKey() {
		return _k;
	}
}
