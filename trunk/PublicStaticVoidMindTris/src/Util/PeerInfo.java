package Util;

import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Vector;

public class PeerInfo {
	private static final int PSEUDO_SIZE = 128; // TODO I may remove the limit of length
	public byte [] _ip;
	public short _port;
	public String _pseudo;
	
	public PeerInfo ( byte[] ip, short port, String pseudo ) {
		_ip = ip;
		_port = port;
		_pseudo = pseudo;
	}
	
	public byte [] toBytes () {
		return toBytes(_ip, _port, _pseudo);
	}
	
	public String toString () {
		try {
			return _pseudo + "(" + ipToString(_ip) + ", " + _port + ")";
		} catch (UnknownHostException e) {
			return _pseudo + "([bad ip], " + _port + ")";
		}
	}
	
	public static String ipToString ( byte[] ip ) throws UnknownHostException {
		return InetAddress.getByAddress(ip).getCanonicalHostName();
	}
	
	public static byte[] toBytes( byte [] ip, short port, String pseudo ) {
		// pseudo : ip : 4 bytes, port : 16-bits (unsigned), pseudo : string(128) 
		byte[] out = new byte [PSEUDO_SIZE+4+2];
		
		System.arraycopy(ip, 0, out, 0, 4);
		out[4] = (byte) (port >>> 8);
		out[4+1] = (byte) port;
		System.arraycopy(pseudo.getBytes(), 0, out, 4+2, pseudo.length());
		
		return out;
	}
	
	public static PeerInfo peerFromBytes ( byte [] data ) throws IOException {
		return readPeerFromBytes(data, 0);
	}

	public static Vector<PeerInfo> peersFromBytes ( byte[] data ) throws IOException {
		Vector<PeerInfo> peers = new Vector<PeerInfo>();
		
		for( int i=0 ; i < data.length ; i += 4+2+PSEUDO_SIZE ) {
			peers.add(readPeerFromBytes(data, i));
		}
		
		
		return peers;
	}
	
	private static PeerInfo readPeerFromBytes ( byte [] data, int offset ) throws IOException {
		byte [] ip = new byte[4];
		short port;
		byte [] pseudo = new byte [PSEUDO_SIZE];
		
		try {
			System.arraycopy(data, offset, ip, 0, 4);
			port = (short) (data[offset + 4] + ( data[offset + 4 + 1] << 8 ));
			System.arraycopy(data, offset + 4 + 2, pseudo, 0, PSEUDO_SIZE);
			
			return new PeerInfo(ip, port, new String(pseudo));
		} catch ( NullPointerException e ) {
			throw new IOException("unable to parse PeerInfo from bytes");
		}
	}
}