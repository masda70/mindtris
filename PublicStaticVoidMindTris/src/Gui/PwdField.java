package Gui;

import java.awt.Color;

import javax.swing.JPasswordField;

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
}
