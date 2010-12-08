package Gui;

import java.awt.Color;

import javax.swing.JLabel;

import Encodings.UString;


public class Lbl extends JLabel {
	private static final long serialVersionUID = 1L;

	public Lbl ( String txt ) {
		super(txt);
		this.setForeground(Color.WHITE);
	}

	public Lbl ( UString txt ) {
		this(txt.v());
	}
}
