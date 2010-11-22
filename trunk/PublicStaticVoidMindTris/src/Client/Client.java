package Client;

import Gui.MainWindow;
import Util.*;

import java.io.*;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.security.*;
import java.security.spec.KeySpec;
import java.security.spec.X509EncodedKeySpec;
import java.util.*;
import java.util.Map.Entry;
import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;


public class Client {
	private static final boolean DEBUG = true;
	private static final short KEY_LEN = 1024;
	public short				_port;
	private MainWindow			_w;
	private CltSrvCh			_srvCh;
	private short				_srvPort;
	private Cipher				_srvCrypter;
	private Cipher				_decrypter;
	private PublicKey			_publicKey;
	private Lobby				_lobby;
	/* some fields to save data during msg exchange */
	private byte				_myId;
	private String				_displayName;
	private int					_lobbyId;
	private long				_sessionId;
	private String				_name;
	private byte[]				_pwd;
	private String				_creator;
	
	public Client ( short srvPort ) {
		_w = new MainWindow(this);
		_srvPort = srvPort;
		
		try {
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
	}

	public void connectToSrv ( String serverIp ) throws IOException {
		_srvCh = new CltSrvCh(serverIp, _srvPort);
		
		SrvHandler srvHdl = new SrvHandler(_srvCh);
		srvHdl.start();
		
		debug("say hello and prtclVersion to the server");
		_srvCh.send(Msg.C_HELLO, Channel.protocolVersion);
	}

	public void createUser ( String name, String displayName, String email, char[] pwd) throws IOException, IllegalBlockSizeException {
		User usr = new User(name,
							displayName.getBytes(Channel.ENCODING),
							email.getBytes(Channel.ENCODING),
							( new String(pwd) ).getBytes(Channel.ENCODING));
		
		_name = name;
		_pwd = usr._pwd;
		_srvCh.send(Msg.CREATE_USER, crypt(usr.toBytes(), _srvCrypter));
	}
	
	public void login ( byte[] _usr, byte[] _pwd ) throws IOException {
		byte usrLen = (byte) _usr.length,
			 pwdLen = (byte) _pwd.length;
		byte[] loginInfo = new byte[1+usrLen+1+pwdLen];
		
		loginInfo[0] = usrLen;
		System.arraycopy(_usr, 0, loginInfo, 1, usrLen);
		loginInfo[1+usrLen] = pwdLen;
		System.arraycopy(_pwd, 0, loginInfo, 1+usrLen+1, pwdLen);
		
		try {
			_srvCh.send(Msg.LOGIN, crypt(loginInfo, _srvCrypter));
		} catch (IllegalBlockSizeException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	public void createLobby ( String name, char[] pwd, int maxPlayers ) throws IOException, IllegalBlockSizeException {
		_name = name;
		_creator = _displayName;
		
		byte[] nameBuf = name.getBytes(Channel.ENCODING);
		byte nameLen = (byte) nameBuf.length;
		boolean pwdRequired;
		byte[] pwdCrypt;
		
		if( pwd.length > 0 ) {
			pwdRequired = true;
			byte[] pwdBuf = ( new String(pwd) ).getBytes(Channel.ENCODING);
			byte pwdLen = (byte) pwdBuf.length;
			byte [] encoded = new byte[1+pwdLen];
			encoded[0] = pwdLen;
			System.arraycopy(pwdBuf, 0, encoded, 1, pwdLen);
			
			_pwd = pwdBuf;
			pwdCrypt = crypt(encoded, _srvCrypter);
		} else {
			pwdRequired = false;
			_pwd = pwdCrypt = new byte[]{0x00};
		}

		int pwdLen = pwdCrypt.length;
		
		int offset = 0;
		byte[] data = new byte[1+nameLen+1+1+pwdLen];
		data[offset++] = nameLen;
		System.arraycopy(nameBuf, 0, data, offset, nameLen);
		offset += nameLen;
		data[offset++] = (byte) maxPlayers;
		data[offset++] = Channel.bool2byte(pwdRequired);
		System.arraycopy(pwdCrypt, 0, data, offset, pwdLen);
		
		_srvCh.send(Msg.CREATE_LOBBY, data);
	}

	public void joinLobby ( int id, String name, String creator, byte [] pwd ) throws IOException {
		_name = name;
		_creator = creator;
		
		byte pwdLen = (byte) pwd.length;
		byte[] pwdCrypt;
		
		if( pwdLen == 1 && pwd[0] == 0x00 ) {
			pwdCrypt = pwd;
		} else {
			try {
				pwdCrypt = crypt(pwd, _srvCrypter);
			} catch (IllegalBlockSizeException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				return;
			}
		}
		
		byte[] encodedKey = _publicKey.getEncoded();
		short keyLen = (short) encodedKey.length;
		
		_srvCh.send(Msg.JOIN_LOBBY,
				Channel.int2bytes(id),
				Channel.short2bytes(_port),
				Channel.short2bytes(keyLen),
				encodedKey,
				pwdCrypt
		);
	}

	public void sendChatMsg ( String msg ) {
		for( Entry<Integer, Peer> o : _lobby._peers ) {
			Peer p = o.getValue();
			
			byte[] msgBuf;
			try {
				msgBuf = msg.getBytes(Channel.ENCODING);
				p.getCh().send(P2PMsg.CHAT_SEND, msgBuf.length, msgBuf);
			} catch (UnsupportedEncodingException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
	
	
/*	public void listenPeers () {
		Thread listen = new Thread( new Runnable () {
			public void run() {
				try {
					ServerSocket serv = new ServerSocket(_myInfo._port);
					
					while( !_stop ) {
						Thread.sleep(10);

						Socket skt = serv.accept();
						Channel ch = new Channel(skt);
						ClientHandler peerHdl = new ClientHandler(ch);
						
						debug("Wait hello message from peer " + skt.getRemoteSocketAddress());
						
						peerHdl.start();
					}
						
				} catch (IOException e) {
					e.printStackTrace();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		});
		
		listen.start();
	}
*/	

	private byte[] crypt ( byte[] data, Cipher crypter ) throws IllegalBlockSizeException {
		try {
			return crypter.doFinal(data);
		} catch (BadPaddingException e) {
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
	
	////////// SRV HANDLERS ///////////
	private class SrvHandler extends MsgHandler<CltSrvCh> {
		public SrvHandler(CltSrvCh ch) {
			super(ch);
			
			addHdl(Msg.S_HELLO, new SrvHelloHdl());
			addHdl(Msg.USR_CREATION, new UsrCreateHdl());
			addHdl(Msg.LOGIN_REPLY, new LoginHdl());
			addHdl(Msg.LOBBY_CREATED, new LobbyCreatedHdl());
			addHdl(Msg.LOBBY_LIST, new LobbyListHdl());
			addHdl(Msg.JOINED_LOBBY, new LobbyJoinedHdl());
		}
	}
	
	private class SrvHelloHdl implements Handler<CltSrvCh> {
		public void handle(byte[] data, CltSrvCh ch) throws IOException {
			switch( data[0] ) {
			case 0x00:
				debug("receive protocol succes.");
				int keyLen = Channel.bytes2short(data, 1);
				
				try {
					KeyFactory fact = KeyFactory.getInstance("RSA");
					byte [] encodedKey = new byte [keyLen];
					System.arraycopy(data, 3, encodedKey, 0, keyLen);
					
					KeySpec keySpec = new X509EncodedKeySpec(encodedKey);
					PublicKey serverKey = fact.generatePublic(keySpec);
					
					_srvCrypter = Cipher.getInstance(Channel.CRYPT_SCHEME);
					_srvCrypter.init(Cipher.ENCRYPT_MODE, serverKey);
					
					debug("receive server public key");
				} catch (Exception e) {
					e.printStackTrace();
				}
				
				_w.printCenter( new String(data, 3+keyLen, data.length-3-keyLen, Channel.ENCODING) );
				break;
			case 0x01:	_w.printError("Wrong protocol");					break;
			default:	_w.printError(new String(data, Channel.ENCODING));
			}
		}
	}
	
	private class UsrCreateHdl implements Handler<CltSrvCh> {
		public void handle(byte[] data, CltSrvCh ch) throws IOException {
			switch( data[0] ) {
			case 0x00:
				_w.print("User created !");
				login(_name.getBytes(Channel.ENCODING), _pwd);
				break;
			case 0x01:	_w.printError("This user already exists");	break;
			case 0x02:	_w.printError("Invalid username");			break;
			case 0x03:	_w.printError("Invalid password");			break;
			case 0x04:	_w.printError("Invalid email");				break;
			default:	System.out.println("Unkown userCreation answer");
			}
		}
	}
	
	private class LoginHdl implements Handler<CltSrvCh> {
		public void handle(byte[] data, CltSrvCh ch) throws IOException {
			switch( data[0] ) {
			case 0x00:
				byte nameLen = data[1];
				byte[] name = new byte[nameLen];
				System.arraycopy(data, 2, name, 0, nameLen);
				_displayName = new String(name, Channel.ENCODING);

				_w.print("connected as " + _displayName);
				
				_srvCh.send(Msg.GET_LOBBY_LIST);
				break;
			case 0x01:	_w.printError("Username doesn't exist");						break;
			case 0x02:	_w.printError("Wrong password");								break;
			case 0x03:	_w.printError("Too many tries");								break;
			case 0x04:	_w.printError("Login, but another instance was disconnected");	break;
			default:	System.out.println("Unknown login answer");
			}
		}
		
	}
	
	private class LobbyCreatedHdl implements Handler<CltSrvCh> {
		public void handle(byte[] data, CltSrvCh ch) throws IOException {
			switch( data[0] ) {
			case 0x00:
				_lobbyId = Channel.bytes2int(data, 1);
				_sessionId = Channel.bytes2long(data, 1+4);
				_w.print("Lobby "+_lobbyId+" created");
				joinLobby(_lobbyId, _name, _creator, _pwd);
				break;
			case 0x01:	_w.printError("invalid password");	break;
			case 0x02:	_w.printError("not enough rights");	break;
			default:
				System.out.println("Unkown lobby created answer");
			}
		}
	}
	
	private class LobbyListHdl implements Handler<CltSrvCh> {
		public void handle(byte[] data, CltSrvCh ch) throws IOException {
			_w.paintLobbyList(Lobby.bytesToList(data));
		}
	}
	
	/**
	 * require _name = LobbyName & _creator = LobbyCreator
	 */
	private class LobbyJoinedHdl implements Handler<CltSrvCh> {
		public void handle ( byte[] data, CltSrvCh ch ) throws IOException {
			switch( data[0] ) {
			case 0x00:
				_myId = data[1];
				Lobby l = new Lobby(_name.getBytes(Channel.ENCODING), _creator.getBytes(Channel.ENCODING), data, 2);
				_lobby = l;
				
				for( Entry<Integer, Peer> o : l._peers ) {
					Peer p = o.getValue();
					String ip = InetAddress.getByAddress(p._ip).getCanonicalHostName();
					P2PCh peerCh = new P2PCh(ip, (short) p._port);
					p.setCh(peerCh);
					peerCh.setPeer(p);
					
					PeerHandler peerHdl = new PeerHandler(peerCh);
					peerHdl.start();
					
					debug("say hello to peer "+ip);
					peerCh.send(P2PMsg.HELLO, _myId, Channel.int2bytes(l._id));
				}
				
				_w.printLobby(l);
				
				/* start to listen peers */
				try {
					ServerSocket srv = new ServerSocket(_port);
					while( true ) {
						Thread.sleep(10);
						
						Socket skt = srv.accept();
						P2PCh peerCh = new P2PCh(skt);
						PeerHandler hdl = new PeerHandler(peerCh);
						
						debug("Wait hello message from peer " + skt.getRemoteSocketAddress());
						
						hdl.start();
					}
				} catch ( IOException e ) {
					e.printStackTrace();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				
	//			break;
			case 0x01:	_w.printError("Wrong password");	break;
			case 0x02:	_w.printError("Lobby is full");		break;
			default:	_w.printError("Unwknown error");
			}
		}
		
	}
	
	
	////////// P2P HANDLERS ///////////
	private class PeerHandler extends MsgHandler<P2PCh> {
		public PeerHandler(P2PCh ch) {
			super(ch);
			
			addHdl(P2PMsg.HELLO, new PeerHelloHdl());
			addHdl(P2PMsg.CHAT_SEND, new ChatHdl());
		}
	}
	
	private class PeerHelloHdl implements Handler<P2PCh> {
		public void handle(byte[] data, P2PCh ch) throws IOException {
			System.out.println("hello");
		}
	}
	

	private class ChatHdl implements Handler<P2PCh> {
		public void handle(byte[] data, P2PCh ch) throws IOException {
			byte len = data[0];
			byte[] msg = new byte[len];
			System.arraycopy(data, 1, msg, 0, len);
			
			Peer sender = ch.getPeer();
			_w.printChatMsg(sender, msg);
		}
		
	}
	
/*	private class TestChatListHandler implements Handler {
		public void handle(byte[] data, Channel ch) throws IOException {
			Vector<PeerInfo> peers = PeerInfo.peersFromBytes(data);
			
			_w.print(peers.size() + " peers connected : ");
			debug("receive list of " + peers.size() + " peers. Start listening peers.");
			
			listenPeers();
			
			for( PeerInfo peer : peers ) {
				try {
					debug("going to add peer " + peer);
					Channel peerCh = new Channel(peer);
					addNewPeer(peerCh, peer);
					
					ClientHandler peerHdl = new ClientHandler(peerCh);	// TODO make a special handler for peer
					peerHdl.start();
					
					_w.print(peer._pseudo + ", ");
					debug("send infos to peer "+peer);
					peerCh.write(new Msg(Msg.C_HELLO, _myInfo.toBytes()));
				} catch ( Exception e ) {
					e.printStackTrace();
					_w.printError("Connexion refused "+peer+"\n" + e.getMessage());
				}
			}
			
			_w.print("\n");
		}
	}
*/	
}
