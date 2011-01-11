package Game;

import java.io.IOException;
import java.util.*;

import Gui.MainWindow;

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
	protected boolean		_stop;

	
	////// CONSTRUCTORS //////
	public Game () {
		_pieces = new LinkedList<Piece>();
		_board = new int[W][H];
		_stop = false;
		
		for( int i=0; i<W; i++ )
			for( int j=0; j<H; j++ )
				_board[i][j] = Piece.EMPTY;
	}

	////// PUBLIC METHODS //////
	public void start( MainWindow gui ) throws IOException {
		_gui = gui;
		_pieceNb = -1;
	}
	
	public Queue<Piece> nextPieces() {
		return _pieces;
	}

	public int pieceNb() {
		return _pieceNb+_pieces.size()+1;
	}
	
	public void addNewPiece ( Piece piece, int pieceOffset ) {
		synchronized (_pieces) {
			_pieces.offer(piece);
			// !!!!! TODO offset
				
		}
	}
	
	public void addMoves ( List<Move> moves ) throws IOException {
		for( Move m : moves ) {
			Piece p = getNextPiece();
			p.setRotation(m.pieceRotation);
			p.addToBoard(_board, m.pieceX-p.offsetX(), m.pieceY+p.offsetY());

			checkLines(m.pieceY+5);
		}
	}
	
	public Piece getFallingPiece () {
		return _currentPiece;
	}

	
	////// GETTER //////
	public int[][] board () {
		return _board;
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
				if( _board[x][y-nbLines] == Piece.EMPTY ) line = false;
			
			if( line ) {
				for( int i=0; i<Game.W; i++ ) {
					for( int j=y-nbLines; j<Game.H-1; j++ )
						_board[i][j] = _board[i][j+1];
					_board[i][Game.H-1] = Piece.EMPTY;
				}
				nbLines ++;
			}
		}
		
		if( _gui != null && nbLines > 0 ) _gui.addScore(nbLines);
	}
	
	protected void gameOver () {
		_stop = true;
		
		if( _gui != null ) _gui.gameOver();
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