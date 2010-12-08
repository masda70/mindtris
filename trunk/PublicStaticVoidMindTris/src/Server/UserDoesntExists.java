package Server;

import Encodings.UString;

public class UserDoesntExists extends Exception {
	private static final long serialVersionUID = 1L;
	
	public UString _usr;
	
	public UserDoesntExists ( UString usr ) {
		_usr = usr;
	}


}
