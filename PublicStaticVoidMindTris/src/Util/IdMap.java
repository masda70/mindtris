package Util;

import java.util.*;
import java.util.Map.Entry;

public class IdMap <O> implements Iterable<Map.Entry<Integer, O>> {
	////// FIELDS //////
	private HashMap<Integer, O> _m;
	private int _size;
	
	////// CONSTRUCTORS //////
	public IdMap () {
		_m = new HashMap<Integer, O>();
		_size = 0;
	}
	
	////// PUBLIC METHODS //////
	public int add ( O o ) {
		int id = getNextId();
		_m.put(id, o);
		_size++;
		return id;
	}
	public void add ( int id, O o ) {
		if( ! _m.containsKey(id) ) {
			_m.put(id, o);
			_size++;
		} else {
			throw new IllegalArgumentException("key already binded");
		}
	}
	
	public O get ( int id ) {
		return _m.get(id);
	}

	public Set<Integer> keys () {
		return _m.keySet();
	}
	public Collection<O> elements () {
		return _m.values();
	}
	
	public void rm ( int id ) {
		_m.remove(id);
		_size--;
	}
	
	public int getNextId () {
		return _size + 1;
	}
	
	public int size () {
		return _size;
	}
	
	public Iterator<Entry<Integer, O>> iterator() {
		return _m.entrySet().iterator();
	}
}
