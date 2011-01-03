package Gui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.GridLayout;
import java.awt.Insets;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.border.Border;


import sun.awt.VerticalBagLayout;

public class GameRight extends JPanel {
	private static final long serialVersionUID = 1L;
	
	public GameRight (JComponent textBox, TxtField chatBar) {
		
		setBackground(Color.BLACK);
		//setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		setLayout(new GridLayout(2, 1));
		
		JPanel adv = new JPanel(new GridLayout(3, 3));
		
		for( int i=0; i<3; i++ ) {
			for( int j=0; j<3; j++ ) {
				JLabel rect = new JLabel();
				rect.setPreferredSize(new Dimension(30, 50));
				rect.setBorder(BorderFactory.createLineBorder(Color.DARK_GRAY, 1));
				adv.add(rect);
			}
		}
		
		add(adv);
		
		JPanel chat = new JPanel(new BorderLayout());
		chat.setBackground(Color.BLACK);
		chat.add(textBox, BorderLayout.CENTER);
		chat.add(chatBar, BorderLayout.SOUTH);
		
		add(chat);
		
	}
	
}
