package Util;

import java.io.IOException;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import Encodings.*;
import IO.*;


public class Lobby implements Encodable {
	////// FIELDS //////
	public int _id,
			   _nbPlayers,
			   _maxPlayers,
			   _myPeerId;
	public UString _name;
	public byte[] _sessionId;
	public AString _pwd;
	public IdMap<Peer> _peers;
	public Peer _creator;
	
	////// CONSTRUCTORS //////
	public Lobby ( int id, UString name, byte[] sessionId, int nbPlayers, int maxPlayers, AString pwd, Peer creator, int creatorId ) {
		_id = id;
		_name = name;
		_sessionId = sessionId;
		_nbPlayers = nbPlayers;
		_maxPlayers = maxPlayers;
		_pwd = pwd;
		_peers = new IdMap<Peer>();
		
		_creator = creator;
		_creator._id = creatorId;
		_peers.add(creatorId, _creator);
	}
	
	public Lobby(int id, UString name, byte[] sessionId, byte nbPlayers, int maxPlayers, AString pwd, Peer creator) {
		_id = id;
		_name = name;
		_sessionId = sessionId;
		_nbPlayers = nbPlayers;
		_maxPlayers = maxPlayers;
		_pwd = pwd;
		_peers = new IdMap<Peer>();
		_creator = creator;
		
		_creator._id = _peers.getNextId();
		_peers.add(creator._id, _creator);
	}
	
	public Lobby ( int id, InData in ) throws IOException {
		_id = id;
		int nameLen = in.readUnsignedByte();
		_name = new UString(in, nameLen);
		_nbPlayers = in.readUnsignedByte();
		int creatorPeerId = in.readUnsignedByte();
		_myPeerId = in.readUnsignedByte();
		_sessionId = new byte[8];
		in.readFully(_sessionId);
		int nbPeers = in.readUnsignedByte();
		_peers = new IdMap<Peer>();
		
		for( int i=0; i<nbPeers; i++ ) {
			Peer p = new Peer(in);
			add(p._id, p);
		}
		
		_creator = _peers.get(creatorPeerId);
	}

	////// ENCODING //////
	public void toBytes ( OutData out ) throws IOException {
		out.writeByte(_name.len());
		out.write(_name);
		out.writeByte(_maxPlayers);
		out.writeByte(_creator._id);
		out.writeByte(_myPeerId);
		out.write(_sessionId);
		out.writeByte(_peers.size());
		
		for( Map.Entry<Integer, Peer> o : _peers ) {
			out.write(o.getValue());
		}
	}
	
	public int len () {
		int len = 1+_name.len()+1+1+1+8+1;
		
		for( Map.Entry<Integer, Peer> o : _peers ) len += o.getValue().len();
		
		return len;
	}

	////// PUBLIC METHODS //////
	public void add ( int id, Peer p ) {
		System.out.println("add " + (p._displayName.v()) + " ("+id+")");
		_peers.add(id, p);
		_nbPlayers++;
	}
	
	public boolean pwdRequired () {
		return _pwd != null;
	}

	public String getCreatorName() {
		return _creator._displayName.v();
	}
	
	////// LIST ENCODING //////
	public static void listToBytes ( IdMap<Lobby> list, OutData out ) throws IOException {
		out.writeByte(list.size());

		for( Map.Entry<Integer, Lobby> o : list ) {
			Lobby l = o.getValue();
			
			out.writeInt(o.getKey());
			out.writeByte(l._name.len());
			out.write(l._name);
			out.writeByte(l._nbPlayers);
			out.writeByte(l._maxPlayers);
			out.writeBoolean(l.pwdRequired());
			out.writeByte(l._creator._displayName.len());
			out.write(l._creator._displayName);
		}
	}

	public static int listEncodingLen(IdMap<Lobby> list) {
		int len = 1;
		
		for( Map.Entry<Integer, Lobby> o : list ) {
			Lobby l = o.getValue();
			len += 4+1+l._name.len()+1+1+1+1+l._creator._displayName.len();
		}
		
		return len;
	}
	
	public static List<Lobby> bytesToList ( InData in ) throws IOException {
		int size = in.readUnsignedByte();
		List<Lobby> list = new LinkedList<Lobby>();
		
		for( short i=0; i<size; i++ ) {
			int id = in.readInt();
			int nameLen = in.readUnsignedByte();
			UString name = new UString(in, nameLen);
			int nbPlayers = in.readUnsignedByte();
			int maxPlayers = in.readUnsignedByte();
			boolean hasPwd = in.readBoolean();
			int creatorLen = in.readUnsignedByte();
			UString creatorName = new UString(in, creatorLen);
			Peer creator = new Peer(0, creatorName, null, 0, null);
			
			AString pwd = hasPwd ? new AString("0") : null;
			
			list.add(new Lobby(id, name, null, nbPlayers, maxPlayers, pwd, creator, 0));
		}
		
		return list;
	}
}
