package Gui;

import java.awt.Color;

import javax.swing.JLabel;


public class Lbl extends JLabel {
	private static final long serialVersionUID = 1L;

	public Lbl ( String txt ) {
		super(txt);
		this.setForeground(Color.WHITE);
	}
}
