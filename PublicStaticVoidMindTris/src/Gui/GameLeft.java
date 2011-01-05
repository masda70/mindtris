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
	private Lbl _scoreLbl;
	private int _score;
	
	public GameLeft ( Queue<Piece> nextPieces ) {
		_nextPieces = nextPieces;
		_nextLabel = new NextLabel();
		_scoreLbl = new Lbl("0");
		_score = 0;
		
		setBackground(Color.BLACK);
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));

		add(new Lbl("Score"));
		add(_scoreLbl);
		add(new Lbl("Next Pieces "));
		add(_nextLabel);
	}

	public void addScore ( int nbLines ) {
		_score += nbLines;
		_scoreLbl.setText(Integer.toString(_score));
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
			
			synchronized( _nextPieces ) {
				Iterator<Piece> iter = _nextPieces.iterator();
				while( iter.hasNext() && y + 3*sz < this.getHeight() ) {
					Piece p = iter.next();
					p.draw(g, sz, x+sz*p.offsetX(), y-sz*p.offsetY(), true);
					y += 3*sz;
				}
			}
		}
	}
}
