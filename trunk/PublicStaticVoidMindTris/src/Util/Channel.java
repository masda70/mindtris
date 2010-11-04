package Util;
import java.io.*;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

public class Channel {
	private static final boolean DEBUG = true;
	public static final byte [] protocolId = {'D','G','M','T'};
	public static final byte [] protocolVersion = {0x00, 0x00, 0x00, 0x01};

	private Socket _s;
	private InputStream _in;
	private OutputStream _out;
	
	public Channel(Socket skt) throws IOException {
		_s = skt;
		_in = _s.getInputStream();
		_out = _s.getOutputStream();
	}

	public Channel ( String ip, int port ) throws UnknownHostException, IOException {
		this(new Socket(ip, port));
	}

	public Channel(PeerInfo infos) throws UnknownHostException, IOException {
		this(PeerInfo.ipToString(infos._ip), infos._port);
	}

	public Msg read () throws IOException {
		byte [] ptclId = new byte[4];
		byte [] lenBuf = new byte[2];
		byte [] typeBuf = new byte[1];

		try {
			while( _in.available() == 0 ) Thread.sleep(10);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		
		if( _in.read(ptclId) != 4
		 || _in.read(lenBuf) != 2
		 || _in.read(typeBuf) != 1 ) {
			throw new IOException("EOF in header");
		}

		short len = (short) (bytes2short(lenBuf) - 7);
		
		byte [] data = new byte[len];
		
		if( _in.read(data) != len ) {
			throw new IOException("EOF before end of message");
		}
		
		if(DEBUG) System.out.println("\t(ch debug)\tReceive from " + _s.getRemoteSocketAddress() + " message : "
				+ byteToString(ptclId)
				+ byteToString(lenBuf)
				+ byteToString(typeBuf)
				+ byteToString(data));
		
		return new Msg(typeBuf[0], data); 
	}

	/*
	 * HEADER:
  	 * -PROTOCOL ID (4 BYTES): Un string, qui sera par defaut "DGMT", encode en ASCII.
  	 * -PACKET LENGTH (2 BYTES): Un entier 16 bit qui donne la taille du paquet tout entier.
  	 * -PACKET ID(1 BYTE): Un entier 8 bit qui encode le type de paquet dont il s'agit.
	 */
	public void write ( Msg m ) throws IOException {		
		_out.write(protocolId);
		_out.write(short2bytes((short) (m.getLen() + 7)));
		_out.write(m.getType());
		_out.write(m.getData());
		
		if(DEBUG) System.out.println("\t(ch debug)\tSend to " + _s.getRemoteSocketAddress() + " message : "
				+ byteToString(protocolId)
				+ byteToString(short2bytes((short) (m.getLen() + 7)))
				+ byteToString(m.getType())
				+ byteToString(m.getData()));
				
		_out.flush();
	}

	public byte [] getIp () {
		return _s.getLocalAddress().getAddress();
	}
	
	public static int bytes2short (byte[]  i) {
//		return i[0] + (i[1] << 8);
		
		ByteBuffer buf = ByteBuffer.wrap(i);
		return buf.getShort();
	}
	
	public static byte [] short2bytes ( short i ) {
		ByteBuffer buf = ByteBuffer.allocate(2);
		buf.putShort(i);
		return buf.array();
	}
	
	public static String byteToString ( byte ... raw ) {
		final String HEX = "0123456789ABCDEF";
		
		if ( raw == null ) return null;
		
		final StringBuilder hex = new StringBuilder( 3 * raw.length );
		for ( final byte b : raw )
			hex.append(HEX.charAt((b & 0xF0) >> 4)).append(HEX.charAt((b & 0x0F))).append(' ');
		
		return hex.toString();
	}
}
