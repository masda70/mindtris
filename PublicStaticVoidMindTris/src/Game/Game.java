package Game;

import java.io.IOException;
import java.util.*;

import Util.IdMap;

import Gui.Board;
import Gui.MainWindow;

public class Game {
	////// STATIC //////
	public static final int W = 10,
							H = 22,
							ROUND_TIME = 100;
	protected static Random rdm = new Random();
	
	////// FIELDS //////
	protected Game			_penaltiesWinner;
	protected Queue<Piece> 	_pieces;
	protected int[][]		_board;
	protected int			_pieceNb;
	protected Piece			_currentPiece;
	protected int			_roundNb;
	protected Board			_display;
	protected IdMap<Integer>_waitingPenalties;
	private int				_rndGen;

	
	////// CONSTRUCTORS //////
	public Game ( int seed ) {
		_pieces = new LinkedList<Piece>();
		_board = new int[W][H];
		_waitingPenalties = new IdMap<Integer>();
		
		for( int i=0; i<W; i++ )
			for( int j=0; j<H; j++ )
				_board[i][j] = Piece.EMPTY;
		
		_rndGen = seed;
	}

	////// PUBLIC METHODS //////
	public void start () throws IOException {
		_pieceNb = -1;
	}

	public void nextRound ( int r ) {
		_roundNb = r;
		
		Integer penalties = _waitingPenalties.get(r);
		if( penalties != null && penalties > 0 ) {
			for( int i=0; i<Game.W; i++ ) {
				for( int j=Game.H-1; j>=0; j-- ) {
					if( j+penalties >= Game.H ) {
						if( _board[i][j] != Piece.EMPTY ) {
							System.out.println("peer game over by direct hit !");
							gameOver();
							return;
						}
					} else {
						_board[i][j+penalties] = _board[i][j];
					}
                }
            }
            
            for( int j=0; j<penalties; j++ ) {
                    int hole = nextHoleInPenalty();
                    
                    for( int i=0; i<Game.W; i++ )
                            _board[i][j] = ( i != hole ) ? Piece.PENALTY : Piece.EMPTY;
            }
            _display.repaint();
		}
		
		_waitingPenalties.rm(r);
	}
	
	public void addNewPiece ( Piece piece, int pieceOffset ) {
		synchronized (_pieces) {
			_pieces.offer(piece);
		}
	}
	
	public void addMoves ( List<Move> moves, int round ) throws IOException {
		for( Move m : moves ) {
			Piece p = getNextPiece();
			p.setRotation(m.pieceRotation);
			
			try {
				System.out.println("adv nb:"+m.pieceNb+" ("+p+") x:"+m.pieceX+" y:"+m.pieceY+" r:"+m.pieceRotation);
				p.addToBoard(_board, m.pieceX-p.offsetX(), m.pieceY+p.offsetY());
			} catch ( IndexOutOfBoundsException e ) {
				gameOver();
			}
			
			int nbLines = checkLines(m.pieceY+5);
			_penaltiesWinner.winPenalties(nbLines, round);
		}
		
		if( moves.size() > 0 ) _display.repaint();
	}
	
	public void winPenalties ( int deletedLines, int initRound ) {
		_waitingPenalties.add(initRound+10, deletedLines - 1);
	}
	
	public void setPenaltiesWinner ( Game winner ) {
		_penaltiesWinner = winner;
	}
	
	public void setBoard ( Board display ) {
		_display = display;
	}
	
	////// GETTER //////
	public Queue<Piece> nextPieces() {
		return _pieces;
	}

	public int pieceNb() {
		return _pieceNb+_pieces.size()+1;
	}
	
	public Piece getFallingPiece () {
		return _currentPiece;
	}

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
	
	protected int checkLines ( int yHigh ) {
		int nbLines = 0;
		int yLow = yHigh-5;
		if( yLow < 0 ) yLow = 0;
		
		for( int y=yLow; y<=yHigh; y++ ) {
			boolean line = true;
			
			for( int x=0; x<Game.W; x++ )
				if( _board[x][y-nbLines] == Piece.EMPTY ) line = false;
			
			if( line ) {
				synchronized( _board ) {
					for( int i=0; i<Game.W; i++ ) {
						for( int j=y-nbLines; j<Game.H-1; j++ )
							_board[i][j] = _board[i][j+1];
						_board[i][Game.H-1] = Piece.EMPTY;
					}
				}
				nbLines ++;
			}
		}
		
		return nbLines;
	}
	
	protected void gameOver () {
	}
	
	protected int nextHoleInPenalty () {
		_rndGen = 18000 * (_rndGen & 65535) + (_rndGen >> 16);
		return _rndGen % Piece.PIECES_NB;
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