package Game;

import java.awt.Color;
import java.awt.Graphics;
import java.io.IOException;

import Encodings.Encodable;
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
	private static final int[][][] KICKS_0 = {
		{{0,0}, {-1,0},	{2,0},	{-1,0},	{2,0}},
		{{-1,0},{0,0},	{0,0},	{0,1},	{0,-2}},
		{{-1,1},{1,1},	{-2,1},	{1,0},	{-2,0}},
		{{0,1}, {0,1},	{0,1},	{0,-1},	{0,2}}
	};
	private static final int[][][] KICKS_12456 = {
		{{0,0},	{0,0},	{0,0},	{0,0},	{0,0}},
		{{0,0},	{1,0},	{1,-1},	{0,2},	{1,2}},
		{{0,0},	{0,0},	{0,0},	{0,0},	{0,0}},
		{{0,0},	{-1,0},	{-1,-1},{0,2},	{-1,2}}
	};
	private static final int [][][] KICKS_3 = {
		 {{0,0}},
		 {{0,-1}},
		 {{-1,-1}},
		 {{-1, 0}}
	};
	
	private static boolean[][][][] PIECES;	// [piece][rotation][col][row]
	private static int[][][] OFFSETS;		// [piece][rotation](i,j)
	private static int[][][][] KICKS;		// [piece][rotation][offsetNb](i,j)
	private static Color[] COLORS = {
		Color.GREEN, Color.BLUE, Color.CYAN, Color.YELLOW, Color.MAGENTA, Color.PINK, Color.RED
	};
	
	static {
		// compute rotations
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
		
		// compute offsets
		OFFSETS = new int[PIECES_NB][4][2];
		for( int p=0; p<PIECES_NB; p++ ) {
			for( int r=0; r<4; r++ ) {
				boolean[][] piece = PIECES[p][r];
				boolean empty = true;
				int len = piece.length,
					iMin=0, jMin=0;

				do {
					int j=0;
					while( empty && j<len ) if( piece[iMin][j++] ) empty = false;
					if( empty ) iMin++;
				} while( empty );
				
				empty = true;
				do {
					int i=0;
					while( empty && i<len ) if( piece[i++][len-jMin-1] ) empty = false;
					if( empty ) jMin++;
				} while( empty );

				OFFSETS[p][r][0] = iMin;
				OFFSETS[p][r][1] = len-1-jMin;
			}
		}
		
		// set kicks
		KICKS = new int[PIECES_NB][][][];
		KICKS[0] = KICKS_0;
		KICKS[1] = KICKS[2] = KICKS[4] = KICKS[5] = KICKS[6] = KICKS_12456;
		KICKS[3] = KICKS_3;
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
	public void draw ( Graphics g, int sz, int x, int y, boolean notGhost ) {
		boolean[][] piece = PIECES[_code][_rotation];
		int len = piece.length;
		
		for( int c=0; c<len; c++ ) {
			for( int r=0; r<len; r++ ) {
				if( piece[c][r] )
					drawSquare(g, _code, sz, x + c*sz, y + r*sz, notGhost );
			}
		}
	}

	public int getRotation () {
		return _rotation;
	}
	
	public int offsetX () {
		return OFFSETS[_code][_rotation][0];
	}
	public int offsetY () {
		return OFFSETS[_code][_rotation][1];
	}
	
	public int spawnX () {
		return ( _code == 0 ) ? -3 : -2;
	}
	public int spawnY () {
		return ( _code == 0 ) ? 1 : 0;
	}
	
	public boolean collide ( int[][] board, int rotation, int x, int y ) {
		boolean[][] piece = PIECES[_code][rotation];
		int len = piece.length;
		
		for( int i=0; i<len; i++ ) {
			for( int j=0; j<len; j++ ) {
				if( piece[i][j]
				 && ( x+i>=Game.W
					|| x+i<0
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

	public boolean collide ( int[][] board, int x, int y ) {
		return collide(board, _rotation, x, y);
	}
	
	public void addToBoard ( int[][] board, int x, int y ) {
		boolean[][] piece = PIECES[_code][_rotation];
		int len = piece.length;
		
		for( int i=0; i<len; i++ )
			for( int j=0; j<len; j++ )
				if( piece[i][j] ) board[x+i][y-j] = _code;
	
	}

	public void setRotation ( int r ) {
		_rotation = r;
	}

	public Kick rotate ( int[][] board, int x, int y ) {
		int rot2 = (_rotation+1) % 4;
		int[][] kick1 = KICKS[_code][_rotation],
				kick2 = KICKS[_code][rot2];
				
		for( int k=0; k<kick1.length; k++ ) {
			int i = kick1[k][0] - kick2[k][0],
				j = kick1[k][1] - kick2[k][1];
			
			if( !collide(board, rot2, x+i, y+j) ) {
				_rotation = rot2;
				return new Kick(i, j);
			}
		}
		
		return null;
	}
	
	////// STATIC //////
	public static void drawSquare(Graphics g, int code, int sz, int x, int y, boolean notGhost) {
		if( y < 0 ) return;
		
		g.setColor(COLORS[code]);
		
		if( notGhost ) g.fillRect(x, y, sz-1, sz-1);  
		else g.drawRect(x, y, sz-2, sz-2);
	}
}
