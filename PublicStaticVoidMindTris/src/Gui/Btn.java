package Gui;

import javax.swing.JButton;

public class Btn extends JButton {
	private static final long serialVersionUID = 1L;
	
	public Btn ( String txt ) {
		super(txt);
	}

	public void addBtnListener ( BtnListener l ) {
		addActionListener(l);
		addKeyListener(l);
	}
}
