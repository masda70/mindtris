package Game;

import java.io.IOException;
import java.util.LinkedList;
import java.util.List;

import Client.Client;
import Gui.MainWindow;

public class ActiveGame extends Game {

	////// FIELDS //////
	private Client 			_c;
	private int				_fallX,
							_fallY,
							_fallenY;
	private List<Move>		_moves;
	
	////// CONSTRUCTORS //////
	public ActiveGame ( Client c ) {
		super();
		
		_c = c;
		_moves = new LinkedList<Move>();
	}
	
	////// PUBLIC METHODS //////
	public void start( MainWindow gui ) throws IOException {
		super.start(gui);
		
		nextFall();
		
		Thread timer = new Thread() {
			public void run() {
				try {
					while( _currentPiece != null ) {
						sleep(1000);
						if( _currentPiece.collide(_board, _fallX, _fallY-1) ) {
							placeCurrent(_fallX, _fallY);
						} else {
							_fallY--;
						}
						_gui.upBoard();
					}
				} catch ( InterruptedException e ) {
					e.printStackTrace();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		};
		timer.start();
	}
	
	public void leftMove() {
		if( !_currentPiece.collide(_board, _fallX-1, _fallY) ) {
			_fallX--;
			computeFall();
		}
	}

	public void rightMove () {
		if( !_currentPiece.collide(_board, _fallX+1, _fallY) ) {
			_fallX++;
			computeFall();
		}
	}

	public void rotate() {
		Kick k = _currentPiece.rotate(_board, _fallX, _fallY);
		if( k != null ) {
			_fallX += k._i;
			_fallY += k._j;
			computeFall();	
		}
	}
	
	public void hardDrop() throws IOException {
		placeCurrent(_fallX, _fallenY);
	}

	public void softDrop() throws IOException {
		if( _currentPiece.collide(_board, _fallX, _fallY-1) ) placeCurrent(_fallX, _fallY);
		else _fallY--;
	}
	
	public List<Move> getMoves() {
		List<Move> ret = _moves;
		
		_moves = new LinkedList<Move>();
		
		return ret;
	}
	

	////// GETTER //////
	public int x () {
		return _fallX;
	}
	public int y () {
		return _fallY;
	}
	public int fallen () {
		return _fallenY;
	}

	
	////// PRIVATE METHODS //////
	protected Piece getNextPiece () throws IOException {
		if( _pieces.size() <= 2 ) _c.askForNewPieces();
		
		return super.getNextPiece();
	}
	private void computeFall () {
		_fallenY = _fallY;
		
		while( !_currentPiece.collide(_board, _fallX, _fallenY-1) )
			_fallenY--;
	}
	
	private void nextFall () throws IOException {
		_currentPiece = getNextPiece();
		
		_fallY = Game.H + _currentPiece.spawnY();
		_fallX = Game.W/2 + _currentPiece.spawnX();
		
		_gui.upNextPieces();
		_gui.upBoard();
		computeFall();
	}
	
	private void placeCurrent ( int x, int y ) throws IOException {
		_currentPiece.addToBoard(_board, x, y);
		checkLines(y);
		
		// TODO check
		int time = (int) System.currentTimeMillis();
		_moves.add(new Move(time, _pieceNb, _currentPiece.getRotation(),
							x+_currentPiece.offsetX(), y+_currentPiece.offsetY()));
		nextFall();
	}
}
