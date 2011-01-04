package Gui;

import Game.*;

import java.awt.Color;
import java.awt.Graphics;
import java.util.*;

import javax.swing.*;


public class GameLeft extends JPanel {
	private static final long serialVersionUID = 1L;
	private NextLabel _nextLabel;
	private Queue<Piece> _nextPieces;
	
	public GameLeft ( Queue<Piece> nextPieces ) {
		_nextPieces = nextPieces;
		_nextLabel = new NextLabel();
		
		setBackground(Color.BLACK);
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));

		add(new Lbl("Score"));
		add(new Lbl("Next Pieces "));
		add(_nextLabel);
	}
	
	private class NextLabel extends JPanel {
		private static final long serialVersionUID = 1L;
		
		public NextLabel () {
			super();
			setBackground(Color.BLACK);
		}
		
		public void paint ( Graphics g ) {
			super.paint(g);
			
			int sz=(this.getWidth() - 10) / 5, x=this.getWidth()/2, y=5;
			
			Iterator<Piece> iter = _nextPieces.iterator();
			while( iter.hasNext() && y + 3*sz < this.getHeight() ) {
				Piece p = iter.next();
				p.draw(g, sz, x+sz*p.offsetX(), y-sz*p.offsetY(), true);
				y += 3*sz;
			}
		}
	}
}
