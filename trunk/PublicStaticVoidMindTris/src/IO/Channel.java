package IO;

import java.io.*;
import java.net.Socket;
import java.net.UnknownHostException;

import Encodings.Ip;

public abstract class Channel {
	////// STATIC //////
	private final boolean DEBUG_IN = true;
	private final boolean DEBUG_OUT = false;
	
	////// FIELDS //////
	protected Socket _s;
	protected Msg _msg;
	private InData _in;
	private OutData _out;
	
	////// CONSTRUCTORS //////
	public Channel(Socket skt) throws IOException {
		_s = skt;
		_in = new InData( DEBUG_IN ?
				new InDebug(_s.getInputStream())
		:	_s.getInputStream());
		_out = new OutData( DEBUG_OUT ?
				new OutDebug(_s.getOutputStream())
			:	_s.getOutputStream());
	}

	public Channel ( String ip, int port ) throws UnknownHostException, IOException {
		this(new Socket(ip, port));
	}

	
	////// OVERRIDE //////
	abstract protected Msg newMsg ( OutData out, int type, int len );
	abstract protected Msg newMsg ( InData in ) throws IOException;
	
	
	////// IO //////
	public Msg read () throws IOException {
		if( DEBUG_IN ) System.out.print("read ");
		Msg m = newMsg(_in);
		if( DEBUG_IN ) System.out.println("end read\n");
		return m;
	}
	
	public void createMsg ( int type, int l ) throws IOException {
		if( DEBUG_OUT ) System.out.print("createMsg ");
		_msg = newMsg(_out, type, l);
	}
	
	public OutData msg () {
		return _msg._out;
	}
	
	public void sendMsg () throws IOException {
		if( DEBUG_OUT ) System.out.println("end sendMsg\n");
		//_out.flush();
		_msg = null;
	}
	
	public void send ( Msg m ) throws IOException {
		if( DEBUG_OUT ) System.out.print("send(M) ");
		_out.write(m._out.getData());
		//_out.flush();
		if( DEBUG_OUT ) System.out.println("end send(M)\n");
	}
	
	public void send ( int type, byte... data ) throws IOException {
		createMsg(type, data.length);
		_out.write(data);
		//_out.flush();
		if( DEBUG_OUT ) System.out.println("end send(...)\n");
	}

	public Ip getIp () {
		return new Ip(_s.getLocalAddress().getAddress());
	}

	////// DEBUG //////
	public static void debug ( byte ... raw ) {
		final String HEX = "0123456789ABCDEF";
		
		if ( raw == null ) {
			System.out.println("null");
			return;
		}
		
		final StringBuilder hex = new StringBuilder( 3 * raw.length );
		for ( final byte b : raw )
			hex.append(HEX.charAt((b & 0xF0) >> 4)).append(HEX.charAt((b & 0x0F))).append(' ');
		
		System.out.println( hex.toString() );
	}
	
	private class OutDebug extends OutputStream {
		private OutputStream _s;
		
		public OutDebug(OutputStream s) {
			_s = s;
		}

		public void write(int b) throws IOException {
			final String HEX = "0123456789ABCDEF";
			
			System.out.print(HEX.charAt((b & 0xF0) >> 4) + "" + HEX.charAt(b & 0x0F) + " ");
			_s.write(b);
		}
	}
	
	private class InDebug extends InputStream {
		private InputStream _s;
		
		public InDebug(InputStream s) {
			_s = s;
		}

		public int read() throws IOException {
			final String HEX = "0123456789ABCDEF";
			int b = _s.read();
			System.out.print(HEX.charAt((b & 0xF0) >> 4) + "" + HEX.charAt(b & 0x0F) + " ");
			return b;
		}

	}
}
