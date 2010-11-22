package Server;

import java.io.*;
import java.util.HashMap;

import Util.User;

public class UsrDataBase {
	private HashMap<String, User> _db;
    
	public UsrDataBase () {
		try {
	    	ObjectInputStream ois = new ObjectInputStream(
					new BufferedInputStream(
					new FileInputStream(
					new File("user_db"))));
			_db = (HashMap<String, User>) ois.readObject();
			ois.close();
		} catch (IOException e) {
			_db = new HashMap<String, User>(5);
			store();
		} catch (ClassNotFoundException e) {
			e.printStackTrace();
		}
	}
	
	public User getInfos ( String usr ) throws UserDoesntExists {
		User i = _db.get(usr);
		if( i == null ) throw new UserDoesntExists(usr);
		return i;
	}

	public void add ( User usr ) throws UserAlreadyExists {
		if( ! _db.containsKey( usr._name ) ) {
			_db.put(usr._name, usr);
			store();
		}
		else throw new UserAlreadyExists();
	}
	
	private void store () {
		try {
			ObjectOutputStream oos = new ObjectOutputStream(
					new BufferedOutputStream(
					new FileOutputStream(
					new File("user_db"))));

			oos.writeObject(_db);
			oos.flush();
			oos.close();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
