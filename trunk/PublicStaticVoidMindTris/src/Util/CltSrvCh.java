package Util;

import java.io.*;
import java.net.Socket;
import java.net.UnknownHostException;

public class CltSrvCh extends Channel {
	protected static final boolean DEBUG = true;
	public static final byte [] protocolId = {'D','G','M','T'};

	private User _usr;
	
	public CltSrvCh ( Socket skt ) throws IOException {
		super(skt);
	}
	
	public CltSrvCh ( String ip, short port ) throws UnknownHostException, IOException {
		super(ip, port);
	}

	public void setUsr ( User usr ) {
		_usr = usr;
	}
	public User getUsr () {
		return _usr;
	}
}
