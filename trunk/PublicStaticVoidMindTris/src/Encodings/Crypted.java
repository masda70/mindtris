package Encodings;

import java.io.IOException;
import java.io.Serializable;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;

import IO.*;
import org.bouncycastle.jce.provider.BouncyCastleProvider;

public class Crypted implements Serializable, Encodable {
	////// STATIC //////
	public static final String CRYPT_SCHEME = "RSA/NONE/OAEPwithSHA1andMGF1Padding";
    public static final String PROVIDER = BouncyCastleProvider.PROVIDER_NAME;
	public static final short KEY_LEN = 1024;
	private static final long serialVersionUID = 1L;
	
	////// FIELDS //////
	private byte[] _encoding;
	
	////// CONSTRUCTORS //////
	public Crypted ( AString raw, Cipher crypter ) throws IOException {
		OutData buf = new OutData(KEY_LEN);
		raw.toBytes(buf);
		
		try {
			_encoding = crypter.doFinal(buf.getData());
		} catch (IllegalBlockSizeException e) {
			e.printStackTrace();
		} catch (BadPaddingException e) {
			e.printStackTrace();
		}
	}
	
	public Crypted ( InData in, int len ) throws IOException {
		_encoding = new byte[len];
		in.readFully(_encoding);
	}

	////// ENCODING //////
	public void toBytes(OutData _out) throws IOException {
		_out.write(_encoding);
	}

	public int len () {
		return _encoding.length;
	}

	////// PUBLIC METHODS //////
	public AString decrypt ( Cipher decrypter ) {
		try {
			return new AString(decrypter.doFinal(_encoding));
		} catch (IllegalBlockSizeException e) {
			e.printStackTrace();
		} catch (BadPaddingException e) {
			e.printStackTrace();
		}
		
		return new AString("ERROR");
	}

}
