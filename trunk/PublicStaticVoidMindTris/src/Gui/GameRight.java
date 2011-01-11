package Gui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.GridLayout;
import java.util.Map;

import javax.swing.BoxLayout;
import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.JScrollPane;

import Game.Game;
import Util.IdMap;
import Util.Peer;


public class GameRight extends JPanel {
	private static final long serialVersionUID = 1L;
	private IdMap<Board> _boards;
	
	public GameRight (IdMap<Game> games, IdMap<Peer> peers, JComponent textBox, TxtField chatBar) {
		setBackground(Color.BLACK);
		setLayout(new GridLayout(2, 1));
		
		int s = games.size(), rows, cols;
		if( s <= 2 ) { rows = 1; cols = 2; }
		else if( s <= 4 ) { rows = 2; cols = 2; }
		else if( s <= 6 ) { rows = 2; cols = 3; }
		else if( s <= 8 ) { rows = 2; cols = 4; }
		else if( s <= 9 ) { rows = 3; cols = 3; }
		else { rows = cols = (int)Math.ceil(Math.sqrt((double)s)); }
		
		JPanel adv = new JPanel(new GridLayout(rows, cols));
		adv.setBackground(Color.BLACK);

		_boards = new IdMap<Board>();

		for( Map.Entry<Integer, Game> o : games ) {
			int peerId = o.getKey();
			Board b = new Board(o.getValue());
			_boards.add(peerId, b);
			
			JPanel advBoard = new JPanel();
			advBoard.setLayout(new BoxLayout(advBoard, BoxLayout.Y_AXIS));
			advBoard.setBackground(Color.BLACK);
			advBoard.add(new Lbl(peers.get(peerId)._displayName));
			advBoard.add(b);
			
			adv.add(advBoard);
		}
		
		add(adv);
		
		JPanel chat = new JPanel(new BorderLayout());
		chat.setBackground(Color.BLACK);
		chat.add(new JScrollPane(textBox), BorderLayout.CENTER);
		chat.add(chatBar, BorderLayout.SOUTH);
		
		add(chat);
		
	}

	public void upPeerBoards( int peerId ) {
		_boards.get(peerId).repaint();
	}
}
