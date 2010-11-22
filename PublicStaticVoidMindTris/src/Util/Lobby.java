package Util;

import java.io.UnsupportedEncodingException;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;


public class Lobby {
	public int _id;
	public byte _nbPlayers,
				_maxPlayers;
	public byte[] _name,
				  _sessionId,
				  _creator,
				  _pwd;
	public IdMap<Peer> _peers;
	
	public Lobby ( int id, byte[] name, byte[] sessionId, byte nbPlayers, byte maxPlayers, byte[] pwd, byte[] creator) {
		_id = id;
		_name = name;
		_sessionId = sessionId;
		_nbPlayers = nbPlayers;
		_maxPlayers = maxPlayers;
		_pwd = pwd;
		_creator = creator;
		_peers = new IdMap<Peer>();
	}

	public Lobby ( byte[] name, byte[] creator, Data d ) {
		_name = name;
		_creator = creator;
		_sessionId = new byte[8];
		d.rd(_sessionId, 8);
		byte nbPeers = d.rdB();
		
		_peers = new IdMap<Peer>();
		for( int i=0; i<nbPeers; i++ ) {
			Peer p = new Peer(d);
			_peers.add(p._id, p);
		}
	}
	
	public byte[][] toBytes ( byte peerId ) {
		int i=0;
		byte len = (byte) _peers.size();
		byte[][] data = new byte[1+1+len][];
				
		data[i] = new byte[1+8];
		data[i][0] = peerId;
		System.arraycopy(_sessionId, 0, data[i], 1, 8);
		i++;
		data[i++] = new byte[]{len};
		
		for( Map.Entry<Integer, Peer> o : _peers ) {
			Peer p = o.getValue();
			data[i] = p.toBytes();
			i++;
		}
		
		return data;
	}

	public String getName() {
		try {
			return new String(_name, Channel.ENCODING);
		} catch ( UnsupportedEncodingException e ) {
			e.printStackTrace();
			return null;
		}
	}

	public String getCreator () {
		try {
			return new String(_creator, Channel.ENCODING);
		} catch ( UnsupportedEncodingException e ) {
			e.printStackTrace();
			return null;
		}
	}
	
	public boolean pwdRequired () {
		return _pwd != null;
	}
	
	public static byte[][] listToBytes ( IdMap<Lobby> list ) {
		byte nb = (byte) list.size();
		byte[][] data = new byte[1+nb][];
		int i = 0;
		
		data[i++] = new byte[]{nb};
		
		for( Map.Entry<Integer, Lobby> o : list ) {
			Lobby l = o.getValue();
			byte nameLen = (byte) l._name.length;
			byte creatorLen = (byte) l._creator.length;
			data[i] = new byte[4+1+nameLen+1+1+1+1+creatorLen];
			
			int offset = 0;
			System.arraycopy(Channel.int2bytes(l._id), 0, data[i], offset, 4);
			offset += 4;
			data[i][offset++] = nameLen;
			System.arraycopy(l._name, 0, data[i], offset, nameLen);
			offset += nameLen;
			data[i][offset++] = l._nbPlayers;
			data[i][offset++] = l._maxPlayers;
			data[i][offset++] = Channel.bool2byte(l.pwdRequired());
			data[i][offset++] = creatorLen;
			System.arraycopy(l._creator, 0, data[i], offset, creatorLen);
			
			i++;
		}
		
		return data;
	}

	public static List<Lobby> bytesToList ( Data d ) {
		int offset = 0;
		short nb = d.rdB();
		List<Lobby> list = new LinkedList<Lobby>();
		
		for( short i=0; i<nb; i++ ) {
			int id = d.rdI();
			byte nameLen = d.rdB();
			byte [] name = new byte[nameLen];
			d.rd(name, nameLen);
			byte nbPlayers = d.rdB();
			byte maxPlayers = d.rdB();
			boolean pwdRequired = d.rdBool();
			byte creatorLen = d.rdB();
			byte[] creator = new byte[creatorLen];
			d.rd(creator, creatorLen);
			
			// TODO /!\ ugly
			byte[] pwd = pwdRequired ? new byte[]{0x01} : null;
			
			list.add(new Lobby(id, name, null, nbPlayers, maxPlayers, pwd, creator));
		}
		
		return list;
	}
}
