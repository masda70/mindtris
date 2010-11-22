package Util;

import java.io.*;
import java.net.Socket;
import java.net.UnknownHostException;

public class P2PCh extends Channel {
	protected static final boolean DEBUG = true;
	public static final byte [] protocolId = {'D','G','M','T','P','2','P'};

	private Peer _p;
	
	public P2PCh ( Socket skt ) throws IOException {
		super(skt);
	}
	
	public P2PCh ( String ip, short port ) throws UnknownHostException, IOException {
		super(ip, port);
	}
	
	public void setPeer ( Peer p ) {
		_p = p;
	}
	
	public Peer getPeer () {
		return _p;
	}
}
