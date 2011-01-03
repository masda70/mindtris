package Game;

import java.util.*;

import Gui.Board;
import Gui.MainWindow;
import Util.*;

public class Game {
	////// STATIC //////
	public static final int W = 10,
							H = 22;
	private static Random rdm = new Random();
	
	////// FIELDS //////
	private Queue<Piece> _pieces;
	private int[][] _board;
	private Piece _currentPiece;
	private int _fallX, _fallY;
	
	////// CONSTRUCTORS //////
	public Game ( Lobby l ) {
		_pieces = new LinkedList<Piece>();
		_board = new int[W][H];
	}

	////// PUBLIC METHODS //////
	public Queue<Piece> nextPieces() {
		return _pieces;
	}
	
	public void addNewPiece ( Piece piece ) {
		_pieces.offer(piece);
	}

	public void start( final MainWindow gui ) {
		_currentPiece = _pieces.poll();

		_fallY = Game.H + _currentPiece.offsetY();
		_fallX = Game.W/2 + _currentPiece.offsetX();
		
		Thread fall = new Thread() {
			public void run() {
				try {
					while( _currentPiece != null ) {
						sleep(1000);
						_fallY--;
						gui.actualize();
					}
				} catch ( InterruptedException e ) {
					e.printStackTrace();
				}
			}
		};
		fall.start();
	}
	
	public Piece getFallingPiece () {
		return _currentPiece;
	}
	
	public int x () {
		return _fallX;
	}
	
	public int y () {
		return _fallY;
	}
	
	////// STATIC FUNCTION //////
	public static Piece[] generateNewPieces () {
		int[] permutation = new int[Piece.PIECES_NB];
		Piece[] newPieces = new Piece[Piece.PIECES_NB];
		
		for( int i=0; i<Piece.PIECES_NB; i++ ) permutation[i]=i;
		
		for( int i=Piece.PIECES_NB-1; i>=0; i-- ) {
			int x = rdm.nextInt(i+1);
			newPieces[i] = new Piece(permutation[x]);
			permutation[x] = permutation[i];
		}
		
		return newPieces;
	}

	public void leftMove() {
		_fallX--;
	}

	public void rightMove () {
		_fallX++;
	}

	public void hardDrop() {
	}

	public void softDrop() {
		
	}

}