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
		}
		
		_gui.upPeerBoards();
	}

	////// PROTECTED //////
	protected Piece getNextPiece () throws IOException {
		_pieceNb++;

		synchronized (_pieces) {
			return _pieces.poll();
		}
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