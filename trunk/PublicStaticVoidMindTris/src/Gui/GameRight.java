package Gui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.GridLayout;
import java.awt.Insets;
import java.awt.LayoutManager;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.border.Border;

import Game.Game;
import Util.IdMap;


import sun.awt.VerticalBagLayout;

public class GameRight extends JPanel {
	private static final long serialVersionUID = 1L;
	private List<Board> _boards;
	
	public GameRight (IdMap<Game> games, JComponent textBox, TxtField chatBar) {
		_boards = new LinkedList<Board>();
		
		for( Map.Entry<Integer, Game> o : games )
			_boards.add(new Board(o.getValue()));
		
		setBackground(Color.BLACK);
		setLayout(new GridLayout(2, 1));
		
		int s = _boards.size(), rows, cols;
		if( s <= 2 ) { rows = 1; cols = 2; }
		else if( s <= 4 ) { rows = 2; cols = 2; }
		else if( s <= 6 ) { rows = 2; cols = 3; }
		else if( s <= 8 ) { rows = 2; cols = 4; }
		else if( s <= 9 ) { rows = 3; cols = 3; }
		else { rows = cols = (int)Math.ceil(Math.sqrt((double)s)); }
		
		JPanel adv = new JPanel(new GridLayout(rows, cols));
		
		//setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		for( Board b : _boards ) adv.add(b);
		
		add(adv);
		
		JPanel chat = new JPanel(new BorderLayout());
		chat.setBackground(Color.BLACK);
		chat.add(textBox, BorderLayout.CENTER);
		chat.add(chatBar, BorderLayout.SOUTH);
		
		add(chat);
		
	}

	public void upPeerBoards() {
		for( JPanel p : _boards ) p.repaint();
	}
}
