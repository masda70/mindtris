package IO;

import java.io.*;
import java.net.Socket;
import java.net.UnknownHostException;

import Util.*;

public class ChP2P extends Channel {
	////// FIELDS //////
	private Peer _p;
	
	////// CONSTRUCTORS //////
	public ChP2P ( Socket skt ) throws IOException {
		super(skt);
	}
	
	public ChP2P ( String ip, int _srvPort ) throws UnknownHostException, IOException {
		super(ip, _srvPort);
	}

	////// PUBLIC METHODS //////
    public void setPeer ( Peer p ) {
        _p = p;
    }
    
    public Peer getPeer () {
        return _p;
    }
    
	////// OVERRIDE METHODS //////
	protected Msg newMsg(OutData out, int type, int len) {
		return new MsgP2P(out, type, len);
	}
	protected Msg newMsg(InData in) throws IOException {
		return new MsgP2P(in);
	}
}