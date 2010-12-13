package Encodings;

import java.io.IOException;
import java.io.Serializable;
import java.math.BigInteger;
import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.PublicKey;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.RSAPublicKeySpec;

import com.sun.org.apache.xml.internal.security.utils.Base64;

import IO.*;

public class RSAKey implements Serializable, Encodable {
	////// STATIC //////
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
			_mod = new byte[modLen];
			in.readFully(_mod);

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

	////// ENCODING //////
	public void toBytes ( OutData out ) throws IOException {
		out.writeShort(_mod.length);
		out.write(_mod);
		out.writeByte(_exp.length);
		out.write(_exp);
	}

	public int len () {
		return 2 + _mod.length + 1 + _exp.length;
	}

	////// PUBLIC METHODS //////
	public PublicKey getKey() {
		return _k;
	}
}
