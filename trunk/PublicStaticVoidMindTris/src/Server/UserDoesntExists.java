package Server;

public class UserDoesntExists extends Exception {
	private static final long serialVersionUID = 1L;
	
	public String _usr;
	
	public UserDoesntExists ( String usr ) {
		_usr = usr;
	}


}
