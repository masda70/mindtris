import java.math.BigInteger;
import java.security.*;
import java.security.spec.DSAPublicKeySpec;
import java.util.Random;

import sun.security.util.DerInputStream;
import sun.security.util.DerValue;

import Client.CreateClient;
import Encodings.DSAKey;
import IO.Channel;
import IO.Msg;
import IO.OutData;
import IO.SignedMsg;

public class Main {

	public static void main(String[] args) {
		CreateClient.main(args);
		
		/* *
		try {
			BigInteger p = new BigInteger("EB73B9D43BF143A4B22134EC9884E049CA85DA7ACA071E6BB9C398277196021FB64120C6793734FBDEA60802E99712E480E16852D708DCD1EA9A43747D39E4C36F7C6E86FD8379C6B2347626FD43B8EB418122880098DFC44F15E6E16C5686B775D7BCD8E4BADCA5624725972A25EB995BDEF43BBE61214CE1648D48D1491D49", 16),
				q = new BigInteger("C5443A073AB8488A414BDB9A3A6B54265FBE80C7", 16),
				g = new BigInteger("BE751D11A0EB852E138C6476995EDBFAB6F483BB7F042F2739BA2576165D30E18A36A8F9BA29BA90B158CB45826ED3D6A63001666087D6398CC3724438B8810A60FD2FE174DE3DC19DFFF4F4FB650618ED9494BC54DD4C49E59DEB09D0405858938ED940C24A8C6DD601E2768154C31EBEE73B7CBAA697A750000FBD4A1ABF17", 16),
				y = new BigInteger("65B51369783F3FFA9E15E3897581474FFF15ACC9F419BE208E0F9C25DEE206F5753F3C4C1D09C898F3E06650212031EE109FC336BD7F4FBD941665C0314A374EFB558CE8CF8A29B070B813ACD2CD1C448FF43BF7EF30D2582111ECE16C661E7F973D0A8E6CB2D126E2EB87D39C07F02E4B271997ACB194B5A01116E5B90CC8A1", 16);
			
			byte[] raw = "lol".getBytes(),
					sign = new BigInteger("47B9CF7CB763F32DB959B1E9BC1349493DCE4954BAD4CCEE4101B9ED73CA0B5C04BC99409F2084EF", 16).toByteArray();
			
			DSAPublicKeySpec spec = new DSAPublicKeySpec(y,p,q,g);
			
			KeyFactory fact = KeyFactory.getInstance("DSA");
			PublicKey _k = fact.generatePublic(spec);

			
			Signature _verifier = Signature.getInstance(SignedMsg.SIGN_SCHEME);
			_verifier.initVerify(_k);
			
			_verifier.update(raw);
			
			System.out.println(_verifier.verify(sign));
			
		} catch ( Exception e ) {
			e.printStackTrace();
		}
		
		/* *
		try {
			KeyPairGenerator gen = KeyPairGenerator.getInstance("DSA");
			gen.initialize(SignedMsg.KEY_LEN);
			KeyPair keyPair = gen.generateKeyPair();
			SecureRandom _rdmGen = SecureRandom.getInstance ("SHA1PRNG");
			
			PrivateKey pr = keyPair.getPrivate();
			PublicKey pu = keyPair.getPublic();
			
			Signature _signer = Signature.getInstance(SignedMsg.SIGN_SCHEME);
			_signer.initSign(pr, _rdmGen);

			byte[] raw = "lol".getBytes();
			
			_signer.update(raw);
			byte[] sign = _signer.sign();


			DerInputStream derIS = new DerInputStream(sign);
			DerValue[] seq = derIS.getSequence(0);
			byte[] r = seq[0].getBigInteger().toByteArray(),
				   s = seq[1].getBigInteger().toByteArray();
			
			Channel.debug(r);
			Channel.debug(s);
			
			DSAKey k = new DSAKey(pu);
			OutData o = new OutData(k.len());
			k.toBytes(o);
			//Channel.debug(o.getData());
			
			Signature _verifier = Signature.getInstance(SignedMsg.SIGN_SCHEME);
			_verifier.initVerify(pu);
			_verifier.update(raw);
			
			System.out.println(_verifier.verify(sign));
			
			
			
		} catch ( Exception e ) {
			e.printStackTrace();
		}
		/* */
	}

}
