package Gui;

import javax.swing.JSpinner;
import javax.swing.SpinnerNumberModel;

public class IntSpinner extends JSpinner {
	private static final long serialVersionUID = 1L;
	private SpinnerNumberModel _model;
	
	public IntSpinner(int init, int min, int max) {
		super();
		
		_model = new SpinnerNumberModel(init, min, max, 1);
		
		this.setModel(_model);
	}

	public int getNb() {
		return _model.getNumber().intValue();
	}

}
