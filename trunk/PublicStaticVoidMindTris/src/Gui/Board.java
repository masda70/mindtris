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
	public void paint ( Graphics g ) {
		super.paint(g);
		int sz = getHeight() / Game.H;
		
		if( _g.getFallingPiece() != null )
			_g.getFallingPiece().draw(g, sz, sz  *_g.x(), sz*(Game.H - _g.y()));
	}
}
