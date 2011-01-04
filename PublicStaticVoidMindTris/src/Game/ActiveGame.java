package Game;

import java.util.LinkedList;
import java.util.List;

import Gui.MainWindow;

public class ActiveGame extends Game {

	////// FIELDS //////
	private int				_fallX,
							_fallY,
							_fallenY;
	private List<Move>		_moves;
	
	////// CONSTRUCTORS //////
	public ActiveGame () {
		super();
		
		_moves = new LinkedList<Move>();
	}
	
	////// PUBLIC METHODS //////
	public void start( MainWindow gui ) {
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
				}
			}
		};
		timer.start();
	}
	
	public void leftMove() {
		if( _fallX + _currentPiece.leftEdgeDist() > 0
		 && !_currentPiece.collide(_board, _fallX-1, _fallY) ) {
			_fallX--;
			computeFall();
		}
	}

	public void rightMove () {
		if( _fallX + _currentPiece.rightEdgeDist() < W
		 && !_currentPiece.collide(_board, _fallX+1, _fallY) ) {
			_fallX++;
			computeFall();
		}
	}

	public void hardDrop() {
		placeCurrent(_fallX, _fallenY);
	}

	public void softDrop() {
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
	private void computeFall () {
		_fallenY = _fallY;
		
		while( !_currentPiece.collide(_board, _fallX, _fallenY-1) )
			_fallenY--;
	}
	
	private void nextFall () {
		_currentPiece = getNextPiece();
		
		_fallY = Game.H + _currentPiece.offsetY();
		_fallX = Game.W/2 + _currentPiece.offsetX();
		
		_gui.upNextPieces();
		_gui.upBoard();
		computeFall();
	}
	
	private void placeCurrent ( int x, int y ) {
		_currentPiece.addToBoard(_board, x, y);
		// TODO check
		int time = (int) System.currentTimeMillis();
		_moves.add(new Move(time, _pieceNb, _currentPiece.getRotation(), x, y));
		nextFall();
	}
}
