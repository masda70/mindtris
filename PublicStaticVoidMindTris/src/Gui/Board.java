package Gui;

import Game.*;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;

import javax.swing.BorderFactory;
import javax.swing.JPanel;


public class Board extends JPanel {
	////// STATIC FIELDS //////
	public static final int BORDER_SZ = 4;
	private static final long serialVersionUID = 1L;

	////// FIELDS //////
	private Game _g;
	private boolean _gameOver;
	
	////// CONSTRUCTORS //////
	public Board ( Game g ) {
		super();
		
		_g = g;
		_gameOver = false;
		setBackground(Color.DARK_GRAY);
		setBorder(BorderFactory.createLineBorder(Color.GRAY, BORDER_SZ-2));
	}

	////// PUBLIC METHODS //////
	public void paint ( Graphics gr ) {
		super.paint(gr);
		int sz = ( getHeight() - 2*BORDER_SZ ) / Game.H;
		
		for( int i=0; i<Game.W; i++ ) {
			for( int j=0; j<Game.H; j++ ) {
				int code = _g.board()[i][j];
				if( code != Piece.EMPTY )
					Piece.drawSquare(gr, code, sz, BORDER_SZ+sz*i, BORDER_SZ+sz*(Game.H-j-1), true); 
			}
		}
		if( _g.getFallingPiece() != null ) {
			ActiveGame g = (ActiveGame)_g;
			g.getFallingPiece().draw(gr, sz, BORDER_SZ+sz*g.x(), BORDER_SZ+sz*(Game.H-g.y()-1), true);
			g.getFallingPiece().draw(gr, sz, BORDER_SZ+sz*g.x(), BORDER_SZ+sz*(Game.H-g.fallen()-1), false);
		}
		
		if( _gameOver ) {
			gr.setColor(Color.GRAY);
            gr.setFont(new Font("arial, helvetica",Font.BOLD, getWidth()/4));
			gr.drawString("GAME", 10, getHeight()/3);
			gr.drawString("OVER", 10, getHeight()/2);
		}
	}

	public void drawGameOver () {
		_gameOver = true;
	}
}
