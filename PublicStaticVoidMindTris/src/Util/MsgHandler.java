package Util;

import java.io.IOException;
import java.util.Hashtable;


public class MsgHandler extends Thread {
	private Channel _ch;
	private Hashtable<Byte, Handler> _handlers;
	private boolean _stop; 
	
	public MsgHandler(Channel ch) {
		_ch = ch;
		_handlers = new Hashtable<Byte, Handler> (20);
		_stop = false;
	}

	public void run () {
		while( ! _stop ) {
			try {
				Msg m = _ch.read();
				byte type = m.getType();
				byte [] data = m.getData();
				
				try {
					_handlers.get(type).handle(data, _ch);
				} catch (NullPointerException e) {
					System.out.println("No handler for type "+type);
					throw e;
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
	
	public void addHandler ( byte type, Handler h ) {
		_handlers.put(type, h);
	}
	
	public void shutDown () {
		_stop = true;
	}
}
