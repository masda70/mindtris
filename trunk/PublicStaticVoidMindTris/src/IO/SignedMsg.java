package IO;

import java.io.IOException;
import java.security.Signature;
import java.security.SignatureException;
import java.security.spec.AlgorithmParameterSpec;
import java.security.spec.PSSParameterSpec;

import Encodings.BigInt;

import sun.security.util.DerInputStream;
import sun.security.util.DerValue;

public class SignedMsg extends MsgP2P {
	////// STATIC //////
	public static final String SIGN_ALGO = "DSA";
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
			byte[] derSign = _signer.sign();

			DerInputStream derIS = new DerInputStream(derSign);
			DerValue[] seq = derIS.getSequence(0);
			BigInt r = new BigInt(seq[0].getBigInteger()),
				   	s = new BigInt(seq[1].getBigInteger());
			
			_length += 2 + r.len() + s.len();
			
			_out.writeShort(r.len() + s.len());
			_out.write(r);
			_out.write(s);
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
