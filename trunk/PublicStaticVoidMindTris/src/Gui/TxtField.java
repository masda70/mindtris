package Gui;

import java.awt.Color;

import javax.swing.JTextField;

import Encodings.*;

public class TxtField extends JTextField {
	private static final long serialVersionUID = 1L;

	public TxtField () {
		super();
		design();
	}

	public TxtField ( int sz ) {
		super(sz);
		design();
	}
	
	public TxtField ( String txt ) {
		super(txt);
		design();
	}
	
	private void design () {
		this.setBackground(Color.BLACK);
		this.setForeground(Color.WHITE);
	}

	public UString getUTxt () {
		return new UString(getText());
	}
	public AString getATxt () {
		return new AString(getText());
	}
}
