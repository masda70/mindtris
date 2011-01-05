package Game;

import java.io.IOException;
import java.util.*;

import Gui.Board;
import Gui.MainWindow;
import Util.*;

public class Game {
	////// STATIC //////
	public static final int W = 10,
							H = 22;
	protected static Random rdm = new Random();
	
	////// FIELDS //////
	protected Queue<Piece> 	_pieces;
	protected int[][]		_board;
	protected int			_pieceNb;
	protected Piece			_currentPiece;
	protected MainWindow	_gui;

	
	////// CONSTRUCTORS //////
	public Game () {
		_pieces = new LinkedList<Piece>();
		_board = new int[W][H];
		
		for( int i=0; i<W; i++ )
			for( int j=0; j<H; j++ )
				_board[i][j] = Piece.EMPTY;
	}

	////// PUBLIC METHODS //////
	public void start( MainWindow gui ) throws IOException {
		_gui = gui;
		_pieceNb = 0;
	}
	
	public Queue<Piece> nextPieces() {
		return _pieces;
	}

	public int pieceNb() {
		return _pieceNb;
	}
	
	public void addNewPiece ( Piece piece, int pieceOffset ) {
		synchronized (_pieces) {
			_pieces.offer(piece);
			// !!!!! TODO offset
				
		}
	}
	
	public Piece getFallingPiece () {
		return _currentPiece;
	}
	
	public void addMoves ( List<Move> moves ) throws IOException {
		for( Move m : moves ) {
			Piece p = getNextPiece();
			p.setRotaion(m.pieceRotation);
			p.addToBoard(_board, m.pieceX, m.pieceY);
			checkLines(m.pieceY);
		}
	}

	////// PROTECTED //////
	protected Piece getNextPiece () throws IOException {
		_pieceNb++;

		synchronized (_pieces) {
			return _pieces.poll();
		}
	}
	
	protected void checkLines ( int yHigh ) {
		int nbLines = 0;
		int yLow = yHigh-5;
		if( yLow < 0 ) yLow = 0;
		
		for( int y=yLow; y<=yHigh; y++ ) {
			boolean line = true;
			
			for( int x=0; x<Game.W; x++ )
				if( _board[x][y] == Piece.EMPTY ) line = false;
			
			if( line ) {
				for( int i=0; i<Game.W; i++ ) {
					for( int j=y; j<Game.H-1; j++ )
						_board[i][j] = _board[i][j+1];
					_board[i][Game.H-1] = Piece.EMPTY;
				}
				nbLines ++;
			}
		}
		
		if( nbLines > 0 ) _gui.addScore(nbLines);
	}
	
	////// GETTER //////
	public int[][] board () {
		return _board;
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

	public static Piece[] generateNewPieces ( int nbPieces ) {
		// !!!! TODO
		return generateNewPieces();
	}
}