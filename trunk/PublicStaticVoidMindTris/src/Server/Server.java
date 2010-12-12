package Server;

import Encodings.*;
import IO.*;
import Util.*;

import java.io.*;
import java.net.*;
import java.security.*;
import java.util.*;
import java.util.Map.Entry;

import javax.crypto.Cipher;
import javax.crypto.NoSuchPaddingException;

public class Server extends Thread {
	////// STATIC //////
	public static final short PORT = 1337+42;
	private static final boolean DEBUG = true;
	private static UString HELLO_MSG = new UString("Welcome to MindTris Server\n");
	
	////// FIELDS //////
	private RSAKey			_publicKey;
	private Cipher			_decrypter;
	private UsrDataBase		_db;
	private IdMap<Lobby>	_lobbies;
	private Random			_rdmGen;
	
	////// CONSTRUCTORS //////
	public Server () {
		_db = new UsrDataBase();
		_lobbies = new IdMap<Lobby>();

		/* generate rsa keys */
		try {
			_rdmGen = SecureRandom.getInstance ("SHA1PRNG");
			KeyPairGenerator gen = KeyPairGenerator.getInstance("RSA");
			gen.initialize(Crypted.KEY_LEN);
			KeyPair keyPair = gen.generateKeyPair();
			_publicKey = new RSAKey( keyPair.getPublic() );
						
			_decrypter = Cipher.getInstance(Crypted.CRYPT_SCHEME);
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
	
	////// PUBLIC METHODS //////
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
				ChCltSrv ch = new ChCltSrv(skt);
				MyHandler hdl = new MyHandler(ch);
				
				debug("Wait hello message from client " + skt.getRemoteSocketAddress());
				
				hdl.start();
			}
		} catch(Exception e) {
			System.out.println("Server error");
			e.printStackTrace();
		}
	}
	
	////// PRIVATE METHODS //////
	private void debug ( String m ) {
		if( DEBUG ) System.out.println(m);
	}
	
	////// HANDLERS //////
	private class MyHandler extends MsgHandler<ChCltSrv> {
		public MyHandler(ChCltSrv ch) {
			super(ch);
			
			addHdl(MsgCltSrv.C_HELLO,			new HelloHdl());
			addHdl(MsgCltSrv.CREATE_USER,		new UsrCreateHdl());
			addHdl(MsgCltSrv.LOGIN,				new LoginHdl());
			addHdl(MsgCltSrv.CREATE_LOBBY,		new LobbyCreateHdl());
			addHdl(MsgCltSrv.GET_LOBBY_LIST,	new LobbyListHdl());
			addHdl(MsgCltSrv.JOIN_LOBBY,		new LobbyJoinHdl());
		}
	}
	
	private class HelloHdl implements Handler<ChCltSrv> {
		public void handle(InData in, ChCltSrv ch) throws IOException {
			byte[] protocolVersion = new byte[4];
			in.readFully(protocolVersion);
			
			if( Arrays.equals(protocolVersion, Msg.protocolVersion) ) {
				debug("Send hello message");
				
				ch.createMsg(MsgCltSrv.S_HELLO, 1 + _publicKey.len() + 2 + HELLO_MSG.len());
				ch.msg().writeByte(0x00);
				ch.msg().write(_publicKey);
				ch.msg().writeShort(HELLO_MSG.len());
				ch.msg().write(HELLO_MSG);
				ch.sendMsg();
			} else {
				debug("Wrong protocol version");
				ch.send(MsgCltSrv.S_HELLO, new byte[]{0x01, 0x00});
			}
		}
	}
	
	private class UsrCreateHdl implements Handler<ChCltSrv> {
		public void handle(InData in, ChCltSrv ch) throws IOException {
			User usr = new User(in, _decrypter);

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
			
			ch.send(MsgCltSrv.USR_CREATION, ans);
		}
	}
	
	private class LoginHdl implements Handler<ChCltSrv> {
		public void handle(InData in, ChCltSrv ch) throws IOException {
			int nameLen = in.readUnsignedByte();
			UString name = new UString(in, nameLen);
			int cryptLen = in.readUnsignedShort();
			Crypted crypt = new Crypted(in, cryptLen);
			AString pwd = crypt.decrypt(_decrypter);
			
			try {
				User usr = _db.getInfos(name);
				ch.setUsr(usr);
				
				if( pwd.equals(usr._pwd) ) {
					ch.createMsg(MsgCltSrv.LOGIN_REPLY, 1+1+usr._displayName.len());
					ch.msg().writeByte(0x00);
					ch.msg().writeByte(usr._displayName.len());
					ch.msg().write(usr._displayName);
					ch.sendMsg();
				} else {
					ch.send(MsgCltSrv.LOGIN_REPLY, (byte)0x02);
				}
			} catch ( UserDoesntExists e ) {
				ch.send(MsgCltSrv.LOGIN_REPLY, (byte)0x01);
			}
		}
	}
	
	private class LobbyCreateHdl implements Handler<ChCltSrv> {
		public void handle(InData in, ChCltSrv ch) throws IOException {
			int nameLen = in.readUnsignedByte();
			UString name = new UString(in, nameLen);
			
			int maxPlayers = in.readUnsignedByte();
			boolean hasPwd = in.readBoolean();

			AString pwd = null;
			
			if( hasPwd ) {
				int cryptedPwdLen = in.readUnsignedShort();
				Crypted cryptedPwd = new Crypted(in, cryptedPwdLen);
				pwd = cryptedPwd.decrypt(_decrypter);
				
				if( !pwd.isValid() ) {
					ch.send(MsgCltSrv.LOBBY_CREATED, (byte)0x01);
					return;
				}
			}
			
			int creatorPort = in.readUnsignedShort();
			DSAKey creatorKey = new DSAKey(in);
			Peer creator = new Peer(0, ch.getUsr()._displayName, ch.getIp(), creatorPort, creatorKey);
			creator.setCh(ch);
			
			int lobbyId = _lobbies.getNextId();
			byte[] nonce = new byte[8];
			_rdmGen.nextBytes(nonce);
			Lobby l = new Lobby(lobbyId, name, nonce, (byte)1, maxPlayers, pwd, creator);
			
			System.out.println(l.getCreatorName());
			_lobbies.add(l);
			
			ch.createMsg(MsgCltSrv.LOBBY_CREATED, 1+4+1+8);
			ch.msg().writeByte(0x00);
			ch.msg().writeInt(lobbyId);
			ch.msg().writeByte(l._creator._id);
			ch.msg().write(nonce);
			ch.sendMsg();
		}
		
	}
	
	private class LobbyListHdl implements Handler<ChCltSrv> {
		public void handle(InData _, ChCltSrv ch) throws IOException {
			ch.createMsg(MsgCltSrv.LOBBY_LIST, Lobby.listEncodingLen(_lobbies));
			Lobby.listToBytes(_lobbies, ch.msg());
			ch.sendMsg();
		}
	}
	
	private class LobbyJoinHdl implements Handler<ChCltSrv> {
		public void handle(InData in, ChCltSrv ch) throws IOException {
			int lobbyId = in.readInt();
			int pwdLen = in.readUnsignedByte();
			AString pwd = null;
			if( pwdLen > 0 ) pwd = new AString(in, pwdLen);
			int port = in.readUnsignedShort();
			DSAKey key = new DSAKey(in);
			
			Lobby l = _lobbies.get(lobbyId);
			
			if( ! l.pwdRequired() || pwd.equals(l._pwd) ) {
				if( l._nbPlayers < l._maxPlayers ) {
					int peerId = l._peers.getNextId();
					l._myPeerId = peerId;
					
					ch.createMsg(MsgCltSrv.JOINED_LOBBY, 4+1+l.len());
					ch.msg().writeInt(lobbyId);
					ch.msg().writeByte(0x00);
					ch.msg().write(l);
					ch.sendMsg();

					User usr = ch.getUsr();
					Peer newPeer = new Peer(peerId, usr._displayName, ch.getIp(), port, key);
					newPeer.setCh(ch);
					
					int len = 4+1+1+1+newPeer._displayName.len()+4+2+newPeer._key.len();
					Msg up = new MsgCltSrv(new OutData(len), MsgCltSrv.UPDATE_CLIENT, len);
					up._out.writeInt(lobbyId);
					up._out.writeByte(0x00);
					up._out.writeByte(newPeer._id);
					up._out.write(newPeer._displayName.len());
					up._out.write(newPeer._displayName);
					up._out.write(newPeer._ip);
					up._out.writeShort(newPeer._port);
					up._out.write(newPeer._key);
					
					for( Entry<Integer, Peer> o : l._peers ) {
						Peer p = o.getValue();
						p.getCh().send(up);
					}

					l.add(peerId, newPeer);
					return;
				} else {
					ch.createMsg(MsgCltSrv.JOINED_LOBBY, 4+1);
					ch.msg().writeInt(lobbyId);
					ch.msg().writeByte(0x02);
					ch.sendMsg();
				}
			} else {
				ch.createMsg(MsgCltSrv.JOINED_LOBBY, 4+1);
				ch.msg().writeInt(lobbyId);
				ch.msg().writeByte(0x01);
				ch.sendMsg();
			}
		}
	}
}