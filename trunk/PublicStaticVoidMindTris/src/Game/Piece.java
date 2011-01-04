package Game;

import java.awt.Color;
import java.awt.Graphics;
import java.io.IOException;
import java.util.Random;

import javax.swing.JComponent;

import Encodings.Encodable;
import IO.InData;
import IO.OutData;

public class Piece implements Encodable {
	////// STATIC //////
	public static final int PIECES_NB = 7;
	public static final int EMPTY = -1;
	private static final long serialVersionUID = 1L;
	private static final String[][] PIECES_STR = {
		{"00000",
		 "00000",
		 "01111",
		 "00000",
		 "00000"},
		{"100",
		 "111",
		 "000"},
		{"001",
		 "111",
		 "000"},
		{"011",
		 "011",
		 "000"},
		{"011",
		 "110",
		 "000"},
		{"010",
		 "111",
		 "000"},
		{"110",
		 "011",
		 "000"}
	};
	private static boolean[][][][] PIECES;	// [piece][rotation][col][row]
	private static Color[] COLORS = {
		Color.GREEN, Color.BLUE, Color.CYAN, Color.YELLOW, Color.MAGENTA, Color.PINK, Color.RED
	};
	
	// compute rotations
	static {
		PIECES = new boolean[PIECES_NB][][][];
		
		for( int p=0; p<PIECES_NB; p++ ) {
			int len = PIECES_STR[p].length;
			
			PIECES[p] = new boolean[4][len][len];
			
			for( int r=0; r<len; r++ ) {
				for( int c=0; c<len; c++ ) {
					boolean b = PIECES_STR[p][r].charAt(c) == '1';

					PIECES[p][0][c][r]				=
					PIECES[p][1][len-r-1][c]		=
					PIECES[p][2][len-c-1][len-r-1]	=
					PIECES[p][3][r][len-c-1]		= b;
				}
			}
		}	
	}
	
	////// FIELDS //////
	private int _code;
	private int _rotation;
	
	////// CONSTRUCTORS //////
	public Piece ( int code ) {
		_code = code;
	}
	
	////// ENCODINGS //////
	public void toBytes(OutData out) throws IOException {
		out.writeByte(_code);
	}
	
	public int len() {
		return 1;
	}
	
	////// PUBLIC MEHTODS //////
	public void draw ( Graphics g, int size, int offsetX, int offsetY, boolean notGhost ) {
		boolean[][] piece = PIECES[_code][_rotation]; 
		int len = piece.length;
		int sz = size;
		
		for( int c=0; c<len; c++ ) {
			for( int r=0; r<len; r++ ) {
				if( piece[c][r] )
					drawSquare(g, _code, sz	, offsetX + c*sz, offsetY + r*sz, notGhost );
			}
		}
	}

	public int getRotation () {
		return _rotation;
	}
	
	public int offsetX () {
		return ( _code == 0 ) ? -3 : -2;
	}
	public int offsetY () {
		return ( _code == 0 ) ? 1 : 0;
	}

	public int leftEdgeDist () {
		return ( _code == 0 || _code == 3 ) ? 1 : 0;
	}
	public int rightEdgeDist () {
		return ( _code == 0 ) ? 5 : 3;
	}

	public boolean collide ( int[][] board, int x, int y ) {
		boolean[][] piece = PIECES[_code][_rotation];
		int len = piece.length;
		
		for( int i=0; i<len; i++ ) {
			for( int j=0; j<len; j++ ) {
				if( piece[i][j]
				 && ( x+i>=Game.W
					|| y-j<0
					|| ( y-j<Game.H && board[x+i][y-j] != EMPTY )
					)
				) {
					return true;
				}
			}
		}
		
		return false;
	}

	public void addToBoard ( int[][] board, int x, int y ) {
		boolean[][] piece = PIECES[_code][_rotation];
		int len = piece.length;
		
		for( int i=0; i<len; i++ )
			for( int j=0; j<len; j++ )
				if( piece[i][j] ) board[x+i][y-j] = _code;
	}

	public void setRotaion ( int r ) {
		_rotation = r;
	}
	
	////// STATIC //////
	public static void drawSquare(Graphics g, int code, int sz, int x, int y, boolean notGhost) {
		g.setColor(COLORS[code]);
		
		if( notGhost ) g.fillRect(x, y, sz-1, sz-1);  
		else g.drawRect(x, y, sz-2, sz-2);
	}
}
