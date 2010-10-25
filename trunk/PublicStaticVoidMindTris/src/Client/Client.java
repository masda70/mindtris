package Client;

import Util.*;

import java.io.*;
import java.net.*;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;


public class Client {
	private static final boolean DEBUG = true;
	private Window _w;
	private Hashtable<Channel, PeerInfo> _peers;
	private Channel _srvCh;
	private PeerInfo _myInfo;
	private short _serverPort;
		
	public Client ( short serverPort ) {
		_w = new Window(this);
		_peers = new Hashtable<Channel, PeerInfo> ();
		_serverPort = serverPort;
	}

	public void connectToSrv ( String serverIp, short myPort, String myPseudo ) throws IOException {
		_srvCh = new Channel(serverIp, _serverPort);
		_myInfo = new PeerInfo(_srvCh.getIp(), myPort, myPseudo);
		
		ClientHandler srvHdl = new ClientHandler(_srvCh);
		srvHdl.start();
		
		debug("say hello and prtclVersion to the server");
		_srvCh.write(new Msg(Msg.C_HELLO, Channel.protocolVersion));
	}
	
	public void listenPeers () {
		Thread listen = new Thread( new Runnable () {
			public void run() {
				try {
					ServerSocket serv = new ServerSocket(_myInfo._port);
					
					while( true ) {
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
	
	public void sendMsg(String msg) throws IOException {
		Enumeration<Channel> channels = _peers.keys();
		
		while ( channels.hasMoreElements() ) {
			channels.nextElement().write(new Msg(Msg.SEND_MSG, msg.getBytes()));
		}
	}
	
	private void addNewPeer( Channel ch, PeerInfo info ) {
		_peers.put(ch, info);
	}
	

	private void debug ( String m ) {
		if( DEBUG ) System.out.println(m);
	}

	////////// HANDLERS ///////////
	private class ClientHandler extends MsgHandler {
		public ClientHandler(Channel ch) {
			super(ch);
			
			addHandler(Msg.S_HELLO, new HelloHandler());
			addHandler(Msg.TEST_CHAT_LIST_PEERS, new TestChatListHandler());
			addHandler(Msg.C_HELLO, new CHelloHandler());
			addHandler(Msg.SEND_MSG, new ChatHandler());
		}
		
	}
	
	private class HelloHandler implements Handler {
		public void handle(byte[] data, Channel ch) throws IOException {
			switch( data[0] ) {
			case 0x00:
				debug("receive protocol succes. Display welcome message.");
				_w.print( new String(data, 1, data.length-1) );
				
				/* */
				debug("ask for the list of peers. Send my info by the way");
				_srvCh.write(new Msg(Msg.TEST_CHAT_LIST_PEERS, _myInfo.toBytes()));
				/* */
				break;
			case 0x01:
				debug("wrong protocol");
				_w.printError("Wrong protocol");
				break;
			default:
				debug("receive unkown error from server");
				for( byte b:data ) _w.printError(b + " ");
				
				_w.printError("Unkown error");
			}
		}
	}
	
	private class TestChatListHandler implements Handler {
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
	
	private class CHelloHandler implements Handler {
		public void handle(byte[] data, Channel ch) throws IOException {
			PeerInfo peer = PeerInfo.peerFromBytes(data);
			
			addNewPeer(ch, peer);
			
			_w.print(peer._pseudo + " has join the chat\n");
		}
	}
	
	private class ChatHandler implements Handler {
		public void handle(byte[] data, Channel ch) throws IOException {
			_w.print(_peers.get(ch)._pseudo + ": " + ( new String(data)) + "\n");
		}
	}
}
