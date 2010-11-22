package Util;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.util.Arrays;

public abstract class Channel {
	protected static final boolean DEBUG = true;
	public static final byte [] protocolId = {'D','G','M','T'};
	public static final byte [] protocolVersion = {0x00, 0x00, 0x00, 0x01};
	public static final String ENCODING = "UTF-8";
	public static final String CRYPT_SCHEME = "RSA/ECB/OAEPwithSHA1andMGF1Padding";

	protected Socket _s;
	protected InputStream _in;
	protected OutputStream _out;
	
	public Channel(Socket skt) throws IOException {
		_s = skt;
		_in = _s.getInputStream();
		_out = _s.getOutputStream();
	}

	public Channel ( String ip, int port ) throws UnknownHostException, IOException {
		this(new Socket(ip, port));
	}

	public Msg read () throws IOException {
		byte [] ptclId = new byte[protocolId.length];
		byte [] lenBuf = new byte[2];
		byte [] typeBuf = new byte[1];

		try {
			while( _in.available() < protocolId.length+2+1 ) Thread.sleep(10);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		
		if(DEBUG) System.out.println("<= "+_s.getRemoteSocketAddress());

		// TODO wait for the end of header if eof
		if( rd(ptclId) != protocolId.length
		 || rd(lenBuf) != 2
		 || rd(typeBuf) != 1 ) {
			throw new IOException("EOF in header");
		}

		if( !Arrays.equals(ptclId, protocolId) )
			throw new IOException("Wrong protocol");
		
		short len = (short) (bytes2short(lenBuf) - 7);
		
		byte [] data = new byte[len];

		try {
			while( _in.available() < len ) Thread.sleep(10);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		if( rd(data) != len ) {
			throw new IOException("EOF before end of message");
		}

		if(DEBUG) System.out.print("\n");
		
		return new Msg(typeBuf[0], data); 
	}

	public void write ( Msg m ) throws IOException {
		if(DEBUG) System.out.println("=> "+_s.getRemoteSocketAddress());

		wr(protocolId);
		wr(short2bytes((short) (m.getLen() + 7)));
		wr(m.getType());
		wr(m.getData());
				
		_out.flush();
		
		if(DEBUG) System.out.print("\n");
	}
	

	public void send ( byte type, byte[] ...data ) throws IOException {
		write( new Msg( type, data ) );
	}
	public void send ( byte type, int ans, byte[] ...data ) throws IOException {
		write( new Msg( type, (byte)ans, data ) );
	}

	public byte [] getIp () {
		return _s.getLocalAddress().getAddress();
	}

	/* ------ byte convertion ------ */
	public static byte bool2byte ( boolean b ) {
		return (byte) (b ? 0x01 : 0x00);
	}
	public static boolean byte2bool ( byte b ) {
		return ( b == 0x00 ) ? false : true;
	}
	
	public static int bytes2short ( byte[] i) {
		return ByteBuffer.wrap(i).getShort();
//		return i[0] + (i[1] << 8);	
	}
	public static int bytes2short ( byte[] i, int offset ) {
		return ByteBuffer.wrap(i, offset, 2).getShort();
	}
	public static int bytes2short ( byte high, byte low ) {
		return (low + (high << 8));
	}
	public static byte[] short2bytes ( short i ) {
		ByteBuffer buf = ByteBuffer.allocate(2);
		buf.putShort(i);
		return buf.array();
	}

	public static int bytes2int ( byte[] i ) {
		return ByteBuffer.wrap(i).getInt();
	}
	public static int bytes2int ( byte[] i, int offset ) {
		return ByteBuffer.wrap(i, offset, 4).getInt();
	}
	public static byte[] int2bytes ( int i ) {
		ByteBuffer buf = ByteBuffer.allocate(4);
		buf.putInt(i);
		return buf.array();
	}

	public static long bytes2long(byte[] data, int i) {
		return ByteBuffer.wrap(data, i, 8).getLong();
	}
	
	/* debug help */
	public static String bytes2string ( byte ... raw ) {
		final String HEX = "0123456789ABCDEF";
		
		if ( raw == null ) return null;
		
		final StringBuilder hex = new StringBuilder( 3 * raw.length );
		for ( final byte b : raw )
			hex.append(HEX.charAt((b & 0xF0) >> 4)).append(HEX.charAt((b & 0x0F))).append(' ');
		
		return hex.toString();
	}
	
	/* ------ overwrite io ------ */
	protected int rd ( byte... buf ) throws IOException {
		int r = _in.read(buf);
		if( DEBUG ) System.out.print(bytes2string(buf));
		return r;
	}
	
	protected void wr ( byte... buf ) throws IOException {
		_out.write(buf);
		if( DEBUG ) System.out.print(bytes2string(buf));
	}
}
