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
import java.util.Map.Entry;
import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;


public class Client {
	private static final boolean DEBUG = true;
	private static final short KEY_LEN = 1024;
	public short				_port = 1337+42+1;
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
	private IdMap<Peer>			_newPeers;
	private IdMap<P2PCh>		_handshakesCh;
	
	public Client ( short srvPort ) {
		_w = new MainWindow(this);
		_srvPort = srvPort;
		_newPeers = new IdMap<Peer>();
		_handshakesCh = new IdMap<P2PCh>();
		
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
		_lobbyId = id;
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

	private byte[] crypt ( byte[] data, Cipher crypter ) throws IllegalBlockSizeException {
		try {
			return crypter.doFinal(data);
		} catch (BadPaddingException e) {
			e.printStackTrace();
		}
		
		return null;
	}

	public void getLobbyList () throws IOException {
		_srvCh.send(Msg.GET_LOBBY_LIST);
	}
	
	protected void addNewPeer ( P2PCh peerCh, Peer p ) {
		debug("add peer " + p.getName());
		
		p.setCh(peerCh);
		peerCh.setPeer(p);
		
		_lobby._peers.add(p);
		_w.printNewPeer(p);
		
		_newPeers.rm(p._id);
		_handshakesCh.rm(p._id);
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
			addHdl(Msg.UPDATE_CLIENT, new UpdateClientHdl());
		}
	}
	
	private class SrvHelloHdl implements Handler<CltSrvCh> {
		public void handle(Data d, CltSrvCh ch) throws IOException {
			switch( d.rdB() ) {
			case 0x00:
				debug("receive protocol succes.");
				int keyLen = d.rdS();
				
				try {
					KeyFactory fact = KeyFactory.getInstance("RSA");
					
					byte[] encodedKey = new byte[keyLen];
					d.rd(encodedKey, keyLen);
					
					KeySpec keySpec = new X509EncodedKeySpec(encodedKey);
					PublicKey serverKey = fact.generatePublic(keySpec);
					
					_srvCrypter = Cipher.getInstance(Channel.CRYPT_SCHEME);
					_srvCrypter.init(Cipher.ENCRYPT_MODE, serverKey);
					
					debug("receive server public key");
				} catch (Exception e) {
					e.printStackTrace();
				}
				
				_w.printCenter(d.rdStr());
				break;
			case 0x01:	_w.printError("Wrong protocol");					break;
			default:	_w.printError(d.rdStr());
			}
		}
	}
	
	private class UsrCreateHdl implements Handler<CltSrvCh> {
		public void handle(Data d, CltSrvCh ch) throws IOException {
			switch( d.rdB() ) {
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
		public void handle(Data d, CltSrvCh ch) throws IOException {
			switch( d.rdB() ) {
			case 0x00:
				byte nameLen = d.rdB();
				_displayName = d.rdStr(nameLen);

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
		public void handle(Data d, CltSrvCh ch) throws IOException {
			switch( d.rdB() ) {
			case 0x00:
				_lobbyId = d.rdI();
				_sessionId = d.rdL();
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
		public void handle(Data d, CltSrvCh ch) throws IOException {
			_w.paintLobbyList(Lobby.bytesToList(d));
		}
	}
	
	private class LobbyJoinedHdl implements Handler<CltSrvCh> {
		public void handle(Data d, CltSrvCh ch) throws IOException {
			switch( d.rdB() ) {
			case 0x00:
				_myId = d.rdB();
				Lobby l = new Lobby(_name.getBytes(Channel.ENCODING), _creator.getBytes(Channel.ENCODING), d);
				l._id = _lobbyId;
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
				
				_w.printLobby(l, _displayName);
				
				/* start to listen peers */
				Thread listenPeers = new Thread() {
					public void run() {
						try {
							ServerSocket srv = new ServerSocket(_port);
							while( true ) {
								Thread.sleep(10);
								
								Socket skt = srv.accept();
								P2PCh peerCh = new P2PCh(skt);
								PeerHandler hdl = new PeerHandler(peerCh);
								
								hdl.start();
							}
						} catch ( IOException e ) {
							e.printStackTrace();
						} catch (InterruptedException e) {
							e.printStackTrace();
						}
					}
				};
				listenPeers.start();
				
				break;
			case 0x01:	_w.printError("Wrong password");	break;
			case 0x02:	_w.printError("Lobby is full");		break;
			default:	_w.printError("Unwknown error");
			}
		}
	}
	
	private class UpdateClientHdl implements Handler<CltSrvCh> {
		public void handle(Data d, CltSrvCh ch) throws IOException {
			switch( d.rdB() ) {
			case 0x00:
				Peer p = new Peer(d);
				P2PCh peerCh = _handshakesCh.get(p._id);
				if( peerCh != null ) {
					addNewPeer(peerCh, p);
				} else {
					_newPeers.add(p._id, p);
					debug(p.getName() + " has join, wait handshakes");
				}
				break;
			default:
				System.out.println("not implemented");
				
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
		public void handle(Data d, P2PCh ch) throws IOException {
			byte id = d.rdB();
			int lobbyId = d.rdI();
			if( lobbyId == _lobby._id ) {
				Peer p = _newPeers.get(id);
				
				if( p != null ) {
					addNewPeer(ch, p);
				} else {
					_handshakesCh.add(id, ch);
					debug("receive handshake from " + InetAddress.getByAddress(ch.getIp()).toString());
				}
			} else {
				System.out.println("new peer with another lobby id");
			}
			
		}
	}
	

	private class ChatHdl implements Handler<P2PCh> {
		public void handle(Data d, P2PCh ch) throws IOException {
			byte len = d.rdB();
			String msg = d.rdStr(len);
			Peer sender = ch.getPeer();
			_w.printChatMsg(sender, msg);
		}
		
	}
}
