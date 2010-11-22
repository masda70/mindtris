package Server;

import Util.*;

import java.io.*;
import java.net.*;
import java.security.*;
import java.util.*;
import java.util.Map.Entry;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;

public class Server extends Thread {
	public static final short PORT = 1337+42;
	private static final boolean DEBUG = true;
	private static final short KEY_LEN = 1024;
	private static String HELLO_MSG = "Welcome to MindTris Server\n";
	
	private PublicKey _publicKey;
	private Cipher _decrypter;
	private UsrDataBase _db;
	private IdMap<Lobby> _lobbies;
	private Random _rdmGen;
	
	public Server () {
		_db = new UsrDataBase();
		_lobbies = new IdMap<Lobby>();

		/* generate rsa keys */
		try {
			_rdmGen = SecureRandom.getInstance ("SHA1PRNG");
			KeyPairGenerator gen = KeyPairGenerator.getInstance("RSA");
			gen.initialize(KEY_LEN);
			KeyPair keyPair = gen.generateKeyPair();
			_publicKey = keyPair.getPublic();
						
			_decrypter = Cipher.getInstance(Channel.CRYPT_SCHEME);
			_decrypter.init(Cipher.DECRYPT_MODE, keyPair.getPrivate());
		} catch (NoSuchAlgorithmException e) {
			e.printStackTrace();
		} catch (InvalidKeyException e) {
			e.printStackTrace();
		} catch (NoSuchPaddingException e) {
			e.printStackTrace();
		}
		
		/* connect to the sql db *
	    try {
	    	String driverName = "org.gjt.mm.mysql.Driver";
			Class.forName(driverName);
		    String url = "jdbc:mysql://" + serverName +  "/" + mydatabase; // a JDBC url

		} catch (ClassNotFoundException e1) {
			e1.printStackTrace();
		}
		
		Connexion connection = DriverManager.getConnection(url, username, password);
	    
	    /* */
	}
	
	public void run () {
		try {
			ServerSocket srv = new ServerSocket(PORT);
			synchronized(this) {
				notifyAll();
			}
			System.out.println("Server launched");
			
			while( true ) {
				Thread.sleep(10);
				
				Socket skt = srv.accept();
				CltSrvCh ch = new CltSrvCh(skt);
				MyHandler hdl = new MyHandler(ch);
				
				debug("Wait hello message from client " + skt.getRemoteSocketAddress());
				
				hdl.start();
			}
		} catch(Exception e) {
			System.out.println("Server error");
			e.printStackTrace();
		}
	}
	
	private byte [] decrypt ( byte [] crypt ) {
		try {
			return _decrypter.doFinal(crypt);
		} catch (IllegalBlockSizeException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (BadPaddingException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		return null;
	}
	
	@SuppressWarnings("unused")
	private void debug ( byte[]... raw ) {
		if( DEBUG )
			for( byte[] b : raw )
				System.out.println(Channel.bytes2string(b));
	}
	private void debug ( String m ) {
		if( DEBUG ) System.out.println(m);
	}
	
	///////// HANDLERS /////////
	private class MyHandler extends MsgHandler<CltSrvCh> {
		public MyHandler(CltSrvCh ch) {
			super(ch);
			
			addHdl(Msg.C_HELLO, new HelloHdl());
			addHdl(Msg.CREATE_USER, new UsrCreateHdl());
			addHdl(Msg.LOGIN, new LoginHdl());
			addHdl(Msg.CREATE_LOBBY, new LobbyCreateHdl());
			addHdl(Msg.GET_LOBBY_LIST, new LobbyListHdl());
			addHdl(Msg.JOIN_LOBBY, new LobbyJoinHdl());
		}
	}
	
	private class HelloHdl implements Handler<CltSrvCh> {
		public void handle(Data d, CltSrvCh ch) throws IOException {
			if( Arrays.equals(d.getBytes(), Channel.protocolVersion) ) {
				debug("Send hello message");
				
				byte[] encodedKey = _publicKey.getEncoded();
				short len = (short) encodedKey.length;
				ch.send(Msg.S_HELLO, 0x00, Channel.short2bytes(len), encodedKey, HELLO_MSG.getBytes(Channel.ENCODING));
			} else {
				debug("Wrong protocol version");
				ch.send(Msg.S_HELLO, new byte[]{0x01, 0x00}, HELLO_MSG.getBytes(Channel.ENCODING));
			}
		}
	}
	
	private class UsrCreateHdl implements Handler<CltSrvCh> {
		public void handle(Data d, CltSrvCh ch) throws IOException {
			User usr = new User (decrypt(d.getBytes()));

			byte ans;
			
			if( ! usr.isNameValid() ) ans = 0x02;
			else if( ! usr.isPwdValid() ) ans = 0x03;
			else if( ! usr.isEmailValid() ) ans = 0x04;
			else {
				try {
					_db.add(usr);
					ans = 0x00;
				} catch (UserAlreadyExists e) {
					ans = 0x01;
				}
			}
			
			ch.send(Msg.USR_CREATION, ans);
		}
	}
	
	private class LoginHdl implements Handler<CltSrvCh> {
		public void handle(Data d, CltSrvCh ch) throws IOException {
			byte[] loginInfo = decrypt(d.getBytes());
				
			byte usrNameLen = loginInfo[0],
				 pwdLen = loginInfo[1+usrNameLen];
			byte [] usrName = new byte [usrNameLen];
			byte [] pwd = new byte [pwdLen];
			
			System.arraycopy(loginInfo, 1, usrName, 0, usrNameLen);
			System.arraycopy(loginInfo, 1+usrNameLen+1, pwd, 0, pwdLen);
			
			byte ans;
			byte displayName [] = {};
			
			try {
				User usr = _db.getInfos(new String(usrName, Channel.ENCODING));
				ch.setUsr(usr);
				
				if( Arrays.equals(usr._pwd, pwd) ) {
					ans = 0x00;
					
					byte dispNameLen = (byte) usr._displayName.length;
					displayName = new byte [2+dispNameLen];
					displayName[0] = dispNameLen;
					System.arraycopy(usr._displayName, 0, displayName, 1, dispNameLen);
				} else {
					ans = 0x02;
				}
			} catch ( UserDoesntExists e ) {
				ans = 0x01;
			}
			
			ch.send(Msg.LOGIN_REPLY, ans, displayName);
		}
	}
	
	private class LobbyCreateHdl implements Handler<CltSrvCh> {
		public void handle(Data d, CltSrvCh ch) throws IOException {
			byte nameLen = d.rdB();
			byte[] name = new byte[nameLen];
			d.rd(name, nameLen);
			byte maxPlayers = d.rdB();
			boolean pwdRequired = d.rdBool();
			byte [] pwd;
			
			byte[] creator = ch.getUsr()._displayName;
			
			if( pwdRequired ) {
				byte[] pwdCrypt = new byte[d.getOutLen()];
				d.rd(pwdCrypt);
				byte[] pwdBuf = decrypt(pwdCrypt);
				int pwdLen = pwdBuf[0];
				pwd = new byte[pwdLen];
				System.arraycopy(pwdBuf, 1, pwd, 0, pwdLen);
				
				// TODO check pwd
				if( pwd == null ) {
					ch.send(Msg.LOBBY_CREATED, 0x01);
					return;
				}
			} else {
				pwd = null;
			}

			int id = _lobbies.getNextId();
			byte[] nonce = new byte[8];
			_rdmGen.nextBytes(nonce);
			_lobbies.add(new Lobby(id, name, nonce, (byte)0, maxPlayers, pwd, creator));
			
			ch.send(Msg.LOBBY_CREATED, 0x00, Channel.int2bytes(id), nonce);
		}
		
	}
	
	private class LobbyListHdl implements Handler<CltSrvCh> {
		public void handle(Data _, CltSrvCh ch) throws IOException {
			ch.send(Msg.LOBBY_LIST, Lobby.listToBytes(_lobbies));
		}
	}
	
	private class LobbyJoinHdl implements Handler<CltSrvCh> {
		public void handle(Data d, CltSrvCh ch) throws IOException {
			int id = d.rdI();
			int port = d.rdS();
			int keyLen = d.rdS();
			byte[] encodedKey = new byte[keyLen];
			d.rd(encodedKey, keyLen);
			byte[] encryptedPwd = new byte[d.getOutLen()];
			d.rd(encryptedPwd);
			
			Lobby l = _lobbies.get(id);
			
			if( ! l.pwdRequired() || Arrays.equals(decrypt(encryptedPwd), l._pwd) ) {
				if( l._nbPlayers < l._maxPlayers ) {
					byte peerId = (byte) _lobbies.getNextId();
					
					ch.send(Msg.JOINED_LOBBY, 0x00, l.toBytes(peerId));
					
					User usr = ch.getUsr();
					Peer newPeer = new Peer(peerId, usr._displayName, ch.getIp(), port, encodedKey);
					newPeer.setCh(ch);
					
					for( Entry<Integer, Peer> o : l._peers ) {
						Peer p = o.getValue();
						// TODO /!\ peerId <-> status_update
						p.getCh().send(Msg.UPDATE_CLIENT, 0x00, newPeer.toBytes());
					}
					
					
					l._peers.add(newPeer);
					
					return;
				} else {
					ch.send(Msg.JOINED_LOBBY, 0x02);
				}
			} else {
				ch.send(Msg.JOINED_LOBBY, 0x01);
			}
		}
	}
}