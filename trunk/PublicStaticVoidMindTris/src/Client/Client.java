package Client;

import Gui.MainWindow;
import Encodings.*;
import IO.*;
import Util.*;

import java.io.*;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;
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
	public int					_port = 1337+42+1;
	private MainWindow			_w;
	private ChCltSrv			_srvCh;
	private int					_srvPort;
	private Cipher				_srvCrypter;
	private Cipher				_decrypter;
	private RSAKey				_publicKey;
	private Lobby				_lobby;
	private int					_lobbyId;
	private long				_sessionId;
	private UString				_name,	
								_displayName;
	private AString				_pwd;
	private IdMap<Peer>			_newPeers;
	private IdMap<ChP2P>		_handshakesCh;
	
	public Client ( short srvPort ) {
		_w = new MainWindow(this);
		_srvPort = srvPort;
		_newPeers = new IdMap<Peer>();
		_handshakesCh = new IdMap<ChP2P>();
		
		try {
			KeyPairGenerator gen = KeyPairGenerator.getInstance("RSA");
			gen.initialize(Crypted.KEY_LEN);
			KeyPair keyPair = gen.generateKeyPair();
			_publicKey = new RSAKey(keyPair.getPublic());
			_decrypter = Cipher.getInstance(Crypted.CRYPT_SCHEME);
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
		_srvCh = new ChCltSrv(serverIp, _srvPort);
		
		SrvHandler srvHdl = new SrvHandler(_srvCh);
		srvHdl.start();
		
		debug("say hello and prtclVersion to the server");
		_srvCh.send(MsgCltSrv.C_HELLO, Msg.protocolVersion);
	}

	public void createUser ( UString name, UString displayName, AString email, AString pwd )
	throws IOException, IllegalBlockSizeException {
		_name = name;
		_pwd = pwd;
		
		User usr = new User(_name, displayName, email, _pwd, _srvCrypter);
		
		_srvCh.createMsg(MsgCltSrv.CREATE_USER, usr.len());
		_srvCh.msg().write(usr);
		_srvCh.sendMsg();
	}
	
	public void login ( UString name, AString pwd ) throws IOException {
		Crypted cryptedPwd = new Crypted(pwd, _srvCrypter);
		
		_srvCh.createMsg(MsgCltSrv.LOGIN, 1 + name.len() + 2 + cryptedPwd.len());
		_srvCh.msg().writeByte(name.len());
		_srvCh.msg().write(name);
		_srvCh.msg().writeShort(cryptedPwd.len());
		_srvCh.msg().write(cryptedPwd);
		_srvCh.sendMsg();
	}

	public void createLobby ( UString name, AString pwd, int maxPlayers )
	throws IOException, IllegalBlockSizeException {
		_pwd = pwd;
		
		int len = 1+name.len()+1+1+2;
		boolean hasPwd = pwd != null;
		Crypted cryptedPwd = null;
		
		if( hasPwd ) {
			cryptedPwd = new Crypted(pwd, _srvCrypter);
			len += cryptedPwd.len();
		} else {
			len ++;
		}
		
		_srvCh.createMsg(MsgCltSrv.CREATE_LOBBY, len);
		_srvCh.msg().writeByte(name.len());
		_srvCh.msg().write(name);
		_srvCh.msg().writeByte(maxPlayers);
		_srvCh.msg().writeBoolean( hasPwd );
		if( hasPwd ) {
			_srvCh.msg().writeShort(cryptedPwd.len());
			_srvCh.msg().write(cryptedPwd);
		} else {
			_srvCh.msg().writeShort(0);
			_srvCh.msg().writeByte(0);
		}
		_srvCh.sendMsg();
	}

	public void getLobbyList () throws IOException {
		_srvCh.send(MsgCltSrv.GET_LOBBY_LIST);
	}
	
	public void joinLobby ( int id, AString pwd ) throws IOException {
		int len = 4+1+2+_publicKey.len();
		
		if( pwd != null ) len += pwd.len();
		
		_srvCh.createMsg(MsgCltSrv.JOIN_LOBBY, len);
		_srvCh.msg().writeInt(id);
		if( pwd != null ) {
			_srvCh.msg().writeByte(pwd.len());
			_srvCh.msg().write(pwd);
		} else {
			_srvCh.msg().writeByte(0x00);
		}
		_srvCh.msg().writeShort(_port);
		_srvCh.msg().write(_publicKey);
		_srvCh.sendMsg();
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

	protected void addNewPeer ( ChP2P peerCh, Peer p ) {
		debug("add peer " + p._displayName.v());
		
		p.setCh(peerCh);
		peerCh.setPeer(p);
		
		_lobby.add(p._id, p);
		_w.printNewPeer(p);
		
		_newPeers.rm(p._id);
		_handshakesCh.rm(p._id);
	}
	
	private void debug ( String m ) {
		if( DEBUG ) System.out.println(m);
	}
	
	////////// SRV HANDLERS ///////////
	private class SrvHandler extends MsgHandler<ChCltSrv> {
		public SrvHandler(ChCltSrv ch) {
			super(ch);
			
			addHdl(MsgCltSrv.S_HELLO, new SrvHelloHdl());
			addHdl(MsgCltSrv.USR_CREATION, new UsrCreateHdl());
			addHdl(MsgCltSrv.LOGIN_REPLY, new LoginHdl());
			addHdl(MsgCltSrv.LOBBY_CREATED, new LobbyCreatedHdl());
			addHdl(MsgCltSrv.LOBBY_LIST, new LobbyListHdl());
			addHdl(MsgCltSrv.JOINED_LOBBY, new LobbyJoinedHdl());
			addHdl(MsgCltSrv.UPDATE_CLIENT, new UpdateClientHdl());
		}
	}
	
	private class SrvHelloHdl implements Handler<ChCltSrv> {
		public void handle(InData in, ChCltSrv ch) throws IOException {
			switch( in.readUnsignedByte() ) {
			case 0x00:
				debug("receive protocol succes.");
				
				try {
					RSAKey encodedKey = new RSAKey(in);
					_srvCrypter = Cipher.getInstance(Crypted.CRYPT_SCHEME);
					_srvCrypter.init(Cipher.ENCRYPT_MODE, encodedKey.getKey());
					
					debug("receive server public key");
				} catch (Exception e) {
					e.printStackTrace();
				}
				
				int motdLen = in.readUnsignedShort();
				UString motd = new UString(in, motdLen);
				_w.printCenter(motd.v());
				break;
			case 0x01:	_w.printError("Wrong protocol");	break;
			default:	_w.printError("Unkown error");
			}
		}
	}
	
	private class UsrCreateHdl implements Handler<ChCltSrv> {
		public void handle(InData in, ChCltSrv ch) throws IOException {
			switch( in.readUnsignedByte() ) {
			case 0x00:
				_w.print("User created !");
				login(_name, _pwd);
				break;
			case 0x01:	_w.printError("This user already exists");	break;
			case 0x02:	_w.printError("Invalid username");			break;
			case 0x03:	_w.printError("Invalid password");			break;
			case 0x04:	_w.printError("Invalid email");				break;
			default:	System.out.println("Unkown userCreation answer");
			}
		}
	}
	
	private class LoginHdl implements Handler<ChCltSrv> {
		public void handle(InData in, ChCltSrv ch) throws IOException {
			switch( in.readUnsignedByte() ) {
			case 0x00:
				int nameLen = in.readUnsignedByte();
				_displayName = new UString(in, nameLen);

				_w.print("connected as " + _displayName.v());
				
				_srvCh.send(MsgCltSrv.GET_LOBBY_LIST);
				break;
			case 0x01:	_w.printError("Username doesn't exist");						break;
			case 0x02:	_w.printError("Wrong password");								break;
			case 0x03:	_w.printError("Too many tries");								break;
			case 0x04:	_w.printError("Login, but another instance was disconnected");	break;
			default:	System.out.println("Unknown login answer");
			}
		}
		
	}
	
	private class LobbyCreatedHdl implements Handler<ChCltSrv> {
		public void handle(InData in, ChCltSrv ch) throws IOException {
			switch( in.readUnsignedByte() ) {
			case 0x00:
				_lobbyId = in.readInt();
				_sessionId = in.readLong();
				joinLobby(_lobbyId, _pwd);
				break;
			case 0x01:	_w.printError("invalid password");	break;
			case 0x02:	_w.printError("not enough rights");	break;
			default:
				System.out.println("Unkown lobby created answer");
			}
		}
	}
	
	private class LobbyListHdl implements Handler<ChCltSrv> {
		public void handle(InData in, ChCltSrv ch) throws IOException {
			_w.paintLobbyList(Lobby.bytesToList(in));
		}
	}
	
	private class LobbyJoinedHdl implements Handler<ChCltSrv> {
		public void handle(InData in, ChCltSrv ch) throws IOException {
			int lobbyId = in.readInt();
			
			switch( in.readUnsignedByte() ) {
			case 0x00:
				_lobby = new Lobby(lobbyId, in);

				for( Entry<Integer, Peer> o : _lobby._peers ) {
					Peer p = o.getValue();
					String ip = p._ip.toString();
					
					try {
						ChP2P peerCh = new ChP2P(ip, (short) p._port);
						
						p.setCh(peerCh);
						peerCh.setPeer(p);
						
						PeerHandler peerHdl = new PeerHandler(peerCh);
						peerHdl.start();
						
						debug("say hello to peer "+ip);
						peerCh.send(P2PMsg.HELLO, _myId, Channel.int2bytes(l._id));
					} catch ( UnknownHostException e ) {
						_w.printError("Unkown host "+ip+"/"+(short)p._port);
					} catch ( IOException e ) {
						_w.printError("IOException for "+ip+"/"+(short)p._port+" :\n"+e.getMessage());
					}
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
	
	private class UpdateClientHdl implements Handler<ChCltSrv> {
		public void handle(InData in, ChCltSrv ch) throws IOException {
			switch( in.readUnsignedByte() ) {
			case 0x00:
				Peer p = new Peer(in);
				ChP2P peerCh = _handshakesCh.get(p._id);
				if( peerCh != null ) {
					addNewPeer(peerCh, p);
				} else {
					_newPeers.add(p._id, p);
					debug(p._displayName.v() + " has join, wait handshakes");
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
