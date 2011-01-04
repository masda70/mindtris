package Gui;

import Game.*;

import java.awt.Color;
import java.awt.Component;
import java.awt.Graphics;

import javax.swing.BorderFactory;
import javax.swing.JPanel;


public class Board extends JPanel {
	////// STATIC FIELDS //////
	private static final long serialVersionUID = 1L;

	////// FIELDS //////
	private Game _g;
	
	////// CONSTRUCTORS //////
	public Board ( Game g ) {
		super();
		
		_g = g;
		setBackground(Color.DARK_GRAY);
		setBorder(BorderFactory.createLineBorder(Color.GRAY, 2));
	}

	////// PUBLIC METHODS //////
	public void paint ( Graphics gr ) {
		super.paint(gr);
		int sz = getHeight() / Game.H;
		
		for( int i=0; i<Game.W; i++ ) {
			for( int j=0; j<Game.H; j++ ) {
				int code = _g.board()[i][j];
				if( code != Piece.EMPTY )
					Piece.drawSquare(gr, code, sz, 2+sz*i, sz*(Game.H-j), true); 
			}
		}
		if( _g.getFallingPiece() != null ) {
			ActiveGame g = (ActiveGame)_g;
			g.getFallingPiece().draw(gr, sz, 2+sz*g.x(), sz*(Game.H - g.y()), true);
			g.getFallingPiece().draw(gr, sz, 2+sz*g.x(), sz*(Game.H - g.fallen()), false);
		}
	}
}
