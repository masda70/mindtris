package Gui;

import java.awt.Color;

import javax.swing.JPasswordField;

import Encodings.AString;

public class PwdField extends JPasswordField {
	private static final long serialVersionUID = 1L;

	public PwdField () {
		super();
		design();
	}
	
	public PwdField ( int sz ) {
		super(sz);
		design();
	}
	
	private void design () {
		this.setBackground(Color.BLACK);
		this.setForeground(Color.WHITE);
	}

	public AString getPwd () {
		char[] pwd = getPassword();
		return ( pwd.length > 0 ) ? new AString(pwd) : null;
	}
}
