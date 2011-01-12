package Client;

import Gui.MainWindow;
import Encodings.*;
import IO.*;
import Util.*;
import Game.*;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;
import java.security.*;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.Map.Entry;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;

import sun.reflect.generics.reflectiveObjects.NotImplementedException;

public class Client {
	////// STATIC //////
	private static final boolean DEBUG = true;
	
	////// FIELDS //////
	public int					_port = 1337+42+1;
	private MainWindow			_w;
	private ChCltSrv			_srvCh;
	private int					_srvPort;
	private ServerSocket		_peerSrvSkt;
	private Cipher				_srvCrypter;
	private Signature			_signer;
	private DSAKey				_publicKey;
	private SecureRandom		_rdmGen;
	private Lobby				_lobby;
	private Peer				_me;
	private UString				_name;
	private AString				_pwd;
	private IdMap<byte[][]>		_challengeCodes;	// [ initCode, lstnCode ]
	private IdMap<ChP2P>		_waitingPeerCh;
	private IdMap<InData>		_waitingAck;
	private ActiveGame			_game;
	private IdMap<Game>			_peerGames;
	private List<Hash>			_hashes;
	private int					_roundNb;
	private List<Integer>		_peersOrder;

	
	////// CONSTRUCTORS //////
	public Client ( short srvPort ) {
		_w = new MainWindow(this);
		_srvPort = srvPort;
		_challengeCodes = new IdMap<byte[][]>();
		_waitingPeerCh = new IdMap<ChP2P>();
		_waitingAck = new IdMap<InData>();
		_peersOrder = new LinkedList<Integer>();
		
		try {
			_rdmGen = SecureRandom.getInstance ("SHA1PRNG");
			KeyPairGenerator gen = KeyPairGenerator.getInstance(SignedMsg.SIGN_ALGO);
			gen.initialize(SignedMsg.KEY_LEN);
			KeyPair keyPair = gen.generateKeyPair();
			_publicKey = new DSAKey(keyPair.getPublic());
			_signer = Signature.getInstance(SignedMsg.SIGN_SCHEME);
			_signer.initSign(keyPair.getPrivate(), _rdmGen);
		//	_signer.setParameter(SignedMsg.SIGN_SPEC);

			_me = new Peer(0, null, null, _port, _publicKey);
		} catch (NoSuchAlgorithmException e) {
			e.printStackTrace();
		} catch (InvalidKeyException e) {
			e.printStackTrace();
		}
	}

	////// PUBLIC METHODS //////
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
		// /!\ temporary lobby
		_lobby = new Lobby(0, name, null, 1, maxPlayers, pwd, _me, 0);
		
		int len = 1+name.len()+1+1+0+2+_publicKey.len();
		boolean hasPwd = pwd != null;
		Crypted cryptedPwd = null;
		
		if( hasPwd ) {
			cryptedPwd = new Crypted(pwd, _srvCrypter);
			len += 2+cryptedPwd.len();
		}
		
		_srvCh.createMsg(MsgCltSrv.CREATE_LOBBY, len);
		_srvCh.msg().writeByte(name.len());
		_srvCh.msg().write(name);
		_srvCh.msg().writeByte(maxPlayers);
		_srvCh.msg().writeBoolean( hasPwd );
		if( hasPwd ) {
			_srvCh.msg().writeShort(cryptedPwd.len());
			_srvCh.msg().write(cryptedPwd);
		}
		_srvCh.msg().writeShort(_port);
		_srvCh.msg().write(_publicKey);
		
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

	public void quitLobby() throws IOException {
		_srvCh.send(MsgCltSrv.LEAVE_LOBBY);
		_lobby = null;
		_peerSrvSkt.close();
		_peerSrvSkt = null;
		
		getLobbyList();
	}
	
	public void startGame () throws IOException {
		_srvCh.send(MsgCltSrv.START_GAME);
	}
	
	public ActiveGame getGame () {
		return _game;
	}

	public IdMap<Game> getPeerGames() {
		return _peerGames;
	}

	public IdMap<Peer> getPeers() {
		return _lobby._peers;
	}
	
	public void sendChatMsg ( UString s ) throws IOException {
		Msg m = new SignedMsg(MsgP2P.CHAT_SEND, 8+2+s.len(), _signer);
		m._out.write(_lobby._sessionId);
		m._out.writeShort(s.len());
		m._out.write(s);
		
		for( Entry<Integer, Peer> o : _lobby._peers ) {
			Peer p = o.getValue();
			System.out.println("PEER "+p._id+" : "+p._displayName.v());
			
			if( p._id != _me._id ) {
				p.getCh().send(m);
			}
		}
	}
	
	public void askForNewPieces () throws IOException {
		_srvCh.createMsg(MsgCltSrv.GIVE_NEW_PIECES, 4+1);
		_srvCh.msg().writeInt(_game.pieceNb());
		_srvCh.msg().writeByte(10);
		_srvCh.sendMsg();
	}

	////// PRIVATE METHODS //////
	private void keepAlive () {
		Thread sendKeepAlive = new Thread () {
			public void run () {
				try {
					Thread.sleep(60*1000);
					_srvCh.send(MsgCltSrv.KEEP_ALIVE_MSG);
				} catch ( IOException e ) {
					e.printStackTrace();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		};
		sendKeepAlive.start();
	}
	
	private void lobbyJoined ( boolean isCreator ) {
		_w.printLobby(_lobby, _me._displayName, isCreator);
		
		/* start to listen peers */
		Thread listenPeers = new Thread() {
			public void run() {
				try {
					_peerSrvSkt = new ServerSocket(_port);
					while( !_peerSrvSkt.isClosed() ) {
						Thread.sleep(10);
						
						Socket skt = _peerSrvSkt.accept();
						ChP2P peerCh = new ChP2P(skt);
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
	}
	
	private void sendConnexionRequest ( Peer p, ChP2P ch ) throws IOException {
		debug("send connexion request to "+p._ip);
		byte[] challengeCode = new byte[8];
		_rdmGen.nextBytes(challengeCode);
		_challengeCodes.add(p._id, new byte[][]{challengeCode, {}});
		
		ch.createMsg(MsgP2P.CONNEXION_REQUEST, 4+1+1+8);
		ch.msg().writeInt(_lobby._id);
		ch.msg().writeByte(_lobby._myPeerId);
		ch.msg().writeByte(p._id);
		ch.msg().write(challengeCode);
		ch.sendMsg();
	}
	
	private void sendConnexionAck ( Peer p, ChP2P ch, InData in ) throws IOException {
		debug("send connexion ack");
		SignedMsg.verify(in, p._verifier);
		ch.setPeer(p);
		
		Msg ans = new SignedMsg(MsgP2P.CONNEXION_ACK, 4+1+1+8+8, _signer);
		ans._out.writeInt(_lobby._id);
		ans._out.writeByte(_lobby._myPeerId);
		ans._out.writeByte(p._id);
		ans._out.write(_challengeCodes.get(p._id)[0]);
		ans._out.write(_challengeCodes.get(p._id)[1]);
		ch.send(ans);
	}
	
	private void verifyConnexionAck ( Peer p, ChP2P ch, InData in ) throws IOException {
		SignedMsg.verify(in, p._verifier);
		ch.setPeer(p);
		p.setCh(ch);

		_w.printNewPeer(p);
	}
	
	public static void debug ( String m ) {
		if( DEBUG ) System.out.println(m);
	}
	
	////// SRV HANDLERS //////
	private class SrvHandler extends MsgHandler<ChCltSrv> {
		public SrvHandler(ChCltSrv ch) {
			super(ch);
			
			addHdl(MsgCltSrv.S_HELLO,			new SrvHelloHdl());
			addHdl(MsgCltSrv.USR_CREATION,		new UsrCreateHdl());
			addHdl(MsgCltSrv.LOGIN_REPLY,		new LoginHdl());
			addHdl(MsgCltSrv.LOBBY_CREATED,		new LobbyCreatedHdl());
			addHdl(MsgCltSrv.LOBBY_LIST,		new LobbyListHdl());
			addHdl(MsgCltSrv.JOINED_LOBBY,		new LobbyJoinedHdl());
			addHdl(MsgCltSrv.UPDATE_CLIENT,		new UpdateClientHdl());
			addHdl(MsgCltSrv.GAME_STARTING,		new GameStartedHdl());
			addHdl(MsgCltSrv.LOAD_GAME,			new GameLoadHdl());
			addHdl(MsgCltSrv.BEGIN_GAME,		new BeginGameHdl());
			addHdl(MsgCltSrv.NEW_PIECES,		new NewPiecesHdl());
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
				
				keepAlive();
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
				_me._displayName = new UString(in, nameLen);
				_w.print("connected as " + _me._displayName.v());
				
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
				int lobbyId = in.readInt();
				int myPeerId = in.readUnsignedByte();
				byte[] sessionId = new byte[8];
				in.readFully(sessionId);
				

				_lobby = new Lobby(lobbyId, _lobby._name, sessionId, _lobby._nbPlayers, _lobby._maxPlayers, _lobby._pwd, _me, myPeerId); 
				_me._id = _lobby._myPeerId = myPeerId;
				
				lobbyJoined(true);
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
				_me._id = _lobby._myPeerId;
				
				for( Entry<Integer, Peer> o : _lobby._peers ) {
					Peer p = o.getValue();
					String ip = p._ip.toString();
					
					try {
						ChP2P peerCh = new ChP2P(ip, p._port);
						
						p.setCh(peerCh);
						peerCh.setPeer(p);
						
						PeerHandler peerHdl = new PeerHandler(peerCh);
						peerHdl.start();
						
						sendConnexionRequest(p, peerCh);
					} catch ( UnknownHostException e ) {
						_w.printError("Unkown host "+ip+"/"+(short)p._port);
					} catch ( IOException e ) {
						_w.printError("IOException for "+ip+"/"+(short)p._port+" :\n"+e.getMessage());
					}
				}
				
				lobbyJoined(false);
				break;
			case 0x01:	_w.printError("Wrong password");	break;
			case 0x02:	_w.printError("Lobby is full");		break;
			default:	_w.printError("Unwknown error");
			}
		}
	}
	
	private class UpdateClientHdl implements Handler<ChCltSrv> {
		public void handle(InData in, ChCltSrv ch) throws IOException {
			int lobbyId = in.readInt();
			
			if( lobbyId != _lobby._id ) {
				debug("update client with another lobby id : "+lobbyId+" instead of "+_lobby._id);
				return;
			}
			
			switch( in.readUnsignedByte() ) {
			case 0x00:
				Peer p = new Peer(in);
				ChP2P waitingPeerCh = _waitingPeerCh.get(p._id);
				
				if( waitingPeerCh == null ) {
					debug(p._displayName.v() + " has join, wait handshakes");
					_lobby._peers.add(p._id, p);
				} else {
					debug("receive update from server");
					InData ack = _waitingAck.get(p._id);
					verifyConnexionAck(p, waitingPeerCh, ack);
					_waitingPeerCh.rm(p._id);
					_waitingAck.rm(p._id);
					_challengeCodes.rm(p._id);
				}
				
				break;
			case 0x01:
				int peerId = in.readUnsignedByte();
				_w.printPeerLeaving(_lobby._peers.get(peerId));
				
				_lobby._peers.rm(peerId);
				break;
			default:
				throw new NotImplementedException();
			}
		}
	}
	
	private class GameStartedHdl implements Handler<ChCltSrv> {
		public void handle(InData in, ChCltSrv ch) throws IOException {
			switch( in.readUnsignedByte() ) {
			case 0x00 :
				debug("game started");
				break;
			default:
				debug("error in game starting");
			}
		}
	}
	
	private class GameLoadHdl implements Handler<ChCltSrv> {
		public void handle(InData in, ChCltSrv ch) throws IOException {
			byte[] buf = _lobby._sessionId;
			int seed = (((buf[4] & 0xff) << 24) | ((buf[5] & 0xff) << 16) | ((buf[6] & 0xff) << 8) | (buf[7] & 0xff));
			debug("seed : "+seed);
			
			_game = new ActiveGame(Client.this, seed);
			_hashes = new LinkedList<Hash>();
			_peerGames = new IdMap<Game>();
			for( int peerId : _lobby._peers.keys() ) {
				if( peerId != _me._id ) _peerGames.add(peerId, new Game(seed));
			}
			
			int nbPieces = in.readUnsignedByte();
			for( int i=0; i<nbPieces; i++ ) {
				int code = in.readUnsignedByte();
				_game.addNewPiece(new Piece(code), 0);
				for( Game g : _peerGames.elements() ) g.addNewPiece(new Piece(code), 0);
			}
			
			int peersOrderSz = in.readUnsignedByte();
			int thrower=-1, winner, first=-1;
			
			for( int i=0; i<peersOrderSz; i++ ) {
				winner = in.readUnsignedByte();
				if( thrower != -1 ) {
					getPeerGames (thrower).setPenaltiesWinner(getPeerGames(winner));
				} else {
					first = winner;
				}
				thrower = winner;
			}
			getPeerGames(thrower).setPenaltiesWinner(getPeerGames(first));
			
			_w.loadGame();
			
			List<Integer> notConnected = new LinkedList<Integer>();
			notConnected.addAll(_challengeCodes.keys());
			// TODO add others cases
			
			if( notConnected.size() > 0 ) {
				_srvCh.createMsg(MsgCltSrv.LOADED_GAME, 1+1+notConnected.size());
				_srvCh.msg().writeByte(0x01);
				_srvCh.msg().writeByte(notConnected.size());
				for( Integer id : notConnected )
					_srvCh.msg().writeByte(id);
			} else {
				_srvCh.send(MsgCltSrv.LOADED_GAME, (byte)0x00);
			}
		}

		private Game getPeerGames ( int id ) {
			return ( id == _me._id ) ? _game : _peerGames.get(id);
		}
	}
	
	private class BeginGameHdl implements Handler<ChCltSrv> {
		public void handle(InData in, ChCltSrv ch) throws IOException {
			debug("Begin Game");
			_game.setGui(_w);
			_game.start();
			for( Game g : _peerGames.elements() ) g.start();
			_w.beginGame();
			
			Thread timer = new Thread() {
				public void run () {
					_roundNb = 0;
					
					try {
						while( true ) {
							sleep(100);
							
							List<Move> moves = _game.getMoves();
							
							SignedMsg msg = new SignedMsg(
									MsgP2P.ROUND,
									8+4+1+moves.size()*Move.encodingLen()+1+_hashes.size()*Hash.encodingLen(),
									_signer);
							msg._out.write(_lobby._sessionId);
							msg._out.writeInt(_roundNb);
							msg._out.writeByte(moves.size());
							for( Move mv : moves ) msg._out.write(mv);
							msg._out.writeByte(_hashes.size());
							for( Hash h : _hashes ) msg._out.write(h);
							
							for( Entry<Integer, Peer> o : _lobby._peers ) {
								Peer p = o.getValue();
								if( p._id != _me._id ) {
									p.getCh().send(msg);
								}
							}
							
							_roundNb++;
						}
					} catch ( InterruptedException e ) {
						e.printStackTrace();
					} catch ( IOException e ) {
						_w.printError(e.getMessage());
					}
				}
			};
			timer.start();
		}
	}
	
	private class NewPiecesHdl implements Handler<ChCltSrv> {
		public void handle(InData in, ChCltSrv ch) throws IOException {
			int pieceOffset = in.readInt();
			int nbPieces = in.readUnsignedByte();

			for( int i=0; i<nbPieces; i++ ) {
				int code = in.readUnsignedByte();
				_game.addNewPiece(new Piece(code), pieceOffset);
				for( Game g : _peerGames.elements() ) g.addNewPiece(new Piece(code), pieceOffset);
			}
			
			_w.upNextPieces();
		}
	}
	
	////// P2P HANDLERS //////
	private class PeerHandler extends MsgHandler<ChP2P> {
		public PeerHandler(ChP2P ch) {
			super(ch);
			
			addHdl(MsgP2P.CONNEXION_REQUEST,	new ConnexionRequestHdl());
			addHdl(MsgP2P.CONNEXION_ACCEPTED,	new ConnexionAcceptedHdl());
			addHdl(MsgP2P.CONNEXION_ACK,		new ConnexionAck());
			addHdl(MsgP2P.CHAT_SEND,			new ChatHdl());
			addHdl(MsgP2P.ROUND, 				new RoundHdl());
		}
	}
	
	private class ConnexionRequestHdl implements Handler<ChP2P> {
		public void handle(InData in, ChP2P ch) throws IOException {
			int lobbyId = in.readInt();
			int peerId = in.readUnsignedByte();
			int myId = in.readUnsignedByte();
			byte[] initChallengeCode = new byte[8];
			in.readFully(initChallengeCode);
			byte[] lsntChallengeCode = new byte[8];
			_rdmGen.nextBytes(lsntChallengeCode);
			_challengeCodes.add(peerId, new byte[][]{initChallengeCode, lsntChallengeCode});
			
			if( lobbyId == _lobby._id && myId == _lobby._myPeerId ) {
				debug("Connexion request ok, send connexion accepted");
				
				Msg ans = new SignedMsg(MsgP2P.CONNEXION_ACCEPTED, 4+1+1+8+8, _signer);
				ans._out.writeInt(_lobby._id);
				ans._out.writeByte(peerId);
				ans._out.writeByte(_lobby._myPeerId);
				ans._out.write(initChallengeCode);
				ans._out.write(lsntChallengeCode);
				ch.send(ans);
			} else {
				System.out.println("connexion req with another lobby id or listening peer id");
				System.out.println("my lobby id : "+_lobby._id+", receive : "+lobbyId);
				System.out.println("my id : "+_lobby._myPeerId+", receive : "+myId);
			}
		}
	}
	
	private class ConnexionAcceptedHdl implements Handler<ChP2P> {
		public void handle(InData in, ChP2P ch) throws IOException {
			int lobbyId = in.readInt();
			int myPeerId = in.readUnsignedByte();
			int peerId = in.readUnsignedByte();
			byte[] initChallengeCode = new byte[8];
			in.readFully(initChallengeCode);
			byte[] lsntChallengeCode = new byte[8];
			in.readFully(lsntChallengeCode);
			
			if( lobbyId == _lobby._id && myPeerId == _lobby._myPeerId ) {
				byte[][] codes = _challengeCodes.get(peerId);
				
				if( Arrays.equals(initChallengeCode, codes[0]) ) {
					codes[1] = lsntChallengeCode;
					
					Peer p = _lobby._peers.get(peerId);
					sendConnexionAck(p, ch, in);
					_challengeCodes.rm(peerId);
				} else {
					System.out.println("connexion acc with wrong init code");
				}
			} else {
				System.out.println("connexion acc with another lobby id or listening peer id");
			}
		}
	}
	
	private class ConnexionAck implements Handler<ChP2P> {
		public void handle(InData in, ChP2P ch) throws IOException {
			int lobbyId = in.readInt();
			int peerId = in.readUnsignedByte();
			int myPeerId = in.readUnsignedByte();
			byte[] initChallengeCode = new byte[8];
			in.readFully(initChallengeCode);
			byte[] lsntChallengeCode = new byte[8];
			in.readFully(lsntChallengeCode);

			if( lobbyId == _lobby._id && myPeerId == _lobby._myPeerId ) {
				byte[][] codes = _challengeCodes.get(peerId);
				
				if( Arrays.equals(initChallengeCode, codes[0]) 
				 && Arrays.equals(lsntChallengeCode, codes[1]) ) {
					
					Peer p = _lobby._peers.get(peerId);
					if( p != null ) {
						debug("connexion ack ok, verify signature");
						verifyConnexionAck(p, ch, in);
						_challengeCodes.rm(peerId);
					} else {
						debug("connexion ack ok, but wait for update status from server");
						_waitingPeerCh.add(peerId, ch);
						_waitingAck.add(peerId, in);
					}
				} else {
					System.out.println("connexion ack with wrong codes");
				}
			} else {
				System.out.println("connexion ack with another lobby id or listening peer id");
			}
		}
	}

	private class ChatHdl implements Handler<ChP2P> {
		public void handle(InData in, ChP2P ch) throws IOException {
			byte[] sessionId = new byte[8];
			in.readFully(sessionId);
			int len = in.readUnsignedShort();
			UString txt = new UString(in, len);
			
			Peer sender = ch.getPeer();
			_w.printChatMsg(sender, txt);
		}
	}
	
	private class RoundHdl implements Handler<ChP2P> {
		public void handle(InData in, ChP2P ch) throws IOException {
			int peerId = ch.getPeer()._id;
			
			byte[] sessionId = new byte[8];
			in.readFully(sessionId);
			if( !Arrays.equals(sessionId, _lobby._sessionId) )
				throw new IOException("session ids differs in game round packet");
			
			int roundNb = in.readInt();
			int movesNb = in.readUnsignedByte();
			List<Move> moves = new LinkedList<Move>();
			for( int i=0; i<movesNb; i++ ) {
				Move m = new Move(in);
				moves.add(m);
			}
			
			int hashesNb = in.readUnsignedByte();
			
			if( roundNb != _roundNb ) {
				debug(Integer.toString(roundNb-_roundNb));
				//debug("Wrong round nb : "+roundNb+" instead of "+_roundNb);
			}
			
			List<Hash> hashes = new LinkedList<Hash>();
			for( int i=0; i<hashesNb; i++ ) hashes.add(new Hash(in));
			
			if( movesNb > 0 ) {
				_peerGames.get(peerId).addMoves(moves);
			}
		}
	}
}
