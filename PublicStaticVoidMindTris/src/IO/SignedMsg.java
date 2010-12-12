package IO;

import java.io.IOException;
import java.security.Signature;
import java.security.SignatureException;
import java.security.spec.AlgorithmParameterSpec;
import java.security.spec.PSSParameterSpec;

public class SignedMsg extends MsgP2P {
	////// STATIC //////
	public static final String SIGN_SCHEME = "SHA1withDSA";
	public static final PSSParameterSpec SIGN_SPEC = new PSSParameterSpec(28);
	public static final int KEY_LEN = 1024;
	
	////// FIELDS //////
	private Signature _signer;
	
	////// CONSTRUCTORS //////
	public SignedMsg ( int type, int len, Signature signer ) {
		_type = type;
		_length = len;
		_out = new OutData(len);
		_signer = signer;
	}
	
	/*public SignedMsg ( InData in ) throws IOException {
		super(in);
	}*/

	////// OVERRIDE //////
	public void end () throws IOException {
		try {
			byte[] data = _out.getData();
			_signer.update(data);
			byte[] sign = _signer.sign();
			_length += 2 + sign.length;
			
			_out.writeShort(sign.length);
			_out.write(sign);
		} catch ( SignatureException e ) {
			throw new IOException("unable to sign message");
		}
	}

	////// STATIC //////
	public static void verify ( InData in, Signature verifier ) throws IOException {
		/*
		int len = in.readUnsignedShort();
		byte[] sign = new byte[len];
		in.readFully(sign);
		byte[] msg = in.g 
		
		verifier.update(sign);
		*/
	}
}
