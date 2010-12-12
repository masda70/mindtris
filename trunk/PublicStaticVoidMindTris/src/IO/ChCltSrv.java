package IO;

import Util.*;

import java.io.*;
import java.net.Socket;
import java.net.UnknownHostException;

public class ChCltSrv extends Channel {
	////// FIELDS //////
	private User _usr;
	
	////// CONSTRUCTORS //////
	public ChCltSrv ( Socket skt ) throws IOException {
		super(skt);
	}
	
	public ChCltSrv ( String ip, int _srvPort ) throws UnknownHostException, IOException {
		super(ip, _srvPort);
	}

	////// PUBLIC METHODS //////
	public void setUsr ( User usr ) {
		_usr = usr;
	}
	
	public User getUsr () {
		return _usr;
	}
	
	////// OVERRIDE METHODS //////
	protected Msg newMsg(OutData out, int type, int len) {
		return new MsgCltSrv(out, type, len);
	}
	protected Msg newMsg(InData in) throws IOException {
		return new MsgCltSrv(in);
	}
}
