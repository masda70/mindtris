package Util;

import java.io.IOException;
import java.util.Hashtable;


public class MsgHandler<C extends Channel> extends Thread {
	private C _ch;
	private Hashtable<Byte, Handler<C>> _handlers;
	private boolean _stop; 
	
	public MsgHandler ( C ch ) {
		_ch = ch;
		_handlers = new Hashtable<Byte, Handler<C>> (20);
		_stop = false;
	}

	public void run () {
		while( ! _stop ) {
			try {
				Msg m = _ch.read();
				byte type = m.getType();
				byte [] data = m.getData();
				
				Handler<C> hdl = _handlers.get(type);
				if( hdl != null ) hdl.handle(data, _ch);
				else System.out.println("No handler for type "+type);				
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
	
	public void addHdl ( byte type, Handler<C> h ) {
		_handlers.put(type, h);
	}
	
	public void shutDown () {
		_stop = true;
	}
}
