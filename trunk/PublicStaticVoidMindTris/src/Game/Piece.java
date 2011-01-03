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
	private static boolean[][][][] PIECES;	// [piece][rotation][row][col]
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

					PIECES[p][0][r][c]				=
					PIECES[p][1][c][len-r-1]		=
					PIECES[p][2][len-r-1][len-c-1]	=
					PIECES[p][3][len-c-1][r]		= b;
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
	public void draw ( Graphics g, int size, int offsetX, int offsetY ) {
		boolean[][] piece = PIECES[_code][_rotation]; 
		int len = piece.length;
		int sz = size;
		
		g.setColor(COLORS[_code]);
		
		for( int r=0; r<len; r++ ) {
			for( int c=0; c<len; c++ ) {
				if( piece[r][c] ) g.fillRect(offsetX + c*sz, offsetY + r*sz, sz-1, sz-1);
			}
		}
	}
	
	public int offsetX () {
		return ( _code == 0 ) ? -3 : -2;
	}
	
	public int offsetY () {
		return ( _code == 0 ) ? 1 : 0;
	}
}
