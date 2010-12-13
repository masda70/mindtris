package Util;

import IO.*;

import java.io.IOException;
import java.util.Hashtable;


public class MsgHandler<C extends Channel> extends Thread {
	private C _ch;
	private Hashtable<Integer, Handler<C>> _handlers;
	private boolean _stop; 
	
	public MsgHandler ( C ch ) {
		_ch = ch;
		_handlers = new Hashtable<Integer, Handler<C>> (20);
		_stop = false;
	}

	public void run () {
		try {
			while( ! _stop ) {
				Msg m = _ch.read();
				int type = m._type;
				InData in = m._in;
				
				Handler<C> hdl = _handlers.get(type);
				if( hdl != null ) hdl.handle(in, _ch);
				else System.out.println("No handler for type "+type);				
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public void addHdl ( int type, Handler<C> h ) {
		_handlers.put(type, h);
	}
	
	public void shutDown () {
		_stop = true;
	}
}
