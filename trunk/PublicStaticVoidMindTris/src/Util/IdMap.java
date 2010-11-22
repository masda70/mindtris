package Util;

import java.util.*;
import java.util.Map.Entry;

public class IdMap <O> implements Iterable<Map.Entry<Integer, O>> {
	private HashMap<Integer, O> _m = new HashMap<Integer, O>();
	
	public int add ( O o ) {
		int id = getNextId();
		_m.put(id, o);
		return id;
	}

	public void add(int id, O o) {
		_m.put(id, o);
	}
	
	public O get ( int id ) {
		return _m.get(id);
	}
	
	public int getNextId () {
		return _m.size() + 1;
	}
	
	public int size () {
		return _m.size();
	}
	
	public Iterator<Entry<Integer, O>> iterator() {
		return _m.entrySet().iterator();
	}
}
