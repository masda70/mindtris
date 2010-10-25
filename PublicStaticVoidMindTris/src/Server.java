
import Util.*;

import java.io.*;
import java.net.*;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.Hashtable;

public class Server {
	public static final short PORT = 1337+42;
	private static final boolean DEBUG = true;
	private static String HELLO_MSG = "Welcome to MindTris Server\n";
	private Hashtable<Channel, PeerInfo> _clients;
	
	public Server () {
		_clients = new Hashtable<Channel, PeerInfo>();
		
		try {
			ServerSocket srvr = new ServerSocket(PORT);
			System.out.println("Server launched");
			
			while( true ) {
				Thread.sleep(10);
				
				Socket skt = srvr.accept();
				Channel ch = new Channel(skt);
				ServerHandler hdl = new ServerHandler(ch);
				
				debug("Wait hello message from client " + skt.getRemoteSocketAddress());
				
				hdl.start();
			}
		} catch(Exception e) {
			System.out.println("Server error");
			e.printStackTrace();
		}
	}

	private void addPeer ( Channel ch, PeerInfo peer ) {
		debug("add " + peer);
		_clients.put(ch, peer);
	}
	

	
	private void debug ( String m ) {
		if( DEBUG ) System.out.println(m);
	}
	
	////////// MAIN ///////////
	public static void main ( String [] args ) {
		new Server ();
	}
	
	///////// HANDLERS /////////
	private class ServerHandler extends MsgHandler {
		public ServerHandler(Channel ch) {
			super(ch);
			
			addHandler(Msg.C_HELLO, new HelloHandler());
			addHandler(Msg.TEST_CHAT_LIST_PEERS, new TestChatListHandler());
		}
	}
	
	private class HelloHandler implements Handler {
		public void handle(byte[] data, Channel ch) throws IOException {
			if( Arrays.equals(data, Channel.protocolVersion) ) {
				debug("Sending protocol success");
				byte [] answer = {0x00};
				ch.write(new Msg(Msg.S_HELLO, answer, HELLO_MSG.getBytes()));
			} else {
				debug("Wrong protocol version");
				byte [] answer = {0x01};
				ch.write(new Msg(Msg.S_HELLO, answer));
			}
		}
	}
	
	private class TestChatListHandler implements Handler {
		public void handle(byte[] data, Channel ch) throws IOException {
			debug("Sending list of peers");
			byte [][] list = new byte[_clients.size()][];
			int i=0;
			
			Enumeration<PeerInfo> peers = _clients.elements(); 
			while( peers.hasMoreElements() ) {
				list[i++] = peers.nextElement().toBytes();
			}
			
			ch.write(new Msg(Msg.TEST_CHAT_LIST_PEERS, list));
			PeerInfo peer = PeerInfo.peerFromBytes(data);
			addPeer(ch, peer);
		}
	}
}