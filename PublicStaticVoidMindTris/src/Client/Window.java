package Client;

import java.awt.*;
import javax.swing.*;

public class Window extends JFrame {
	private static final long serialVersionUID = 1L;
	
	private Client _c;
	private JTextField _ip, _port, _pseudo, _msg;
	private JTextArea _ta;
	private JButton _connect;
	
	public Window ( Client c ) {		
		super("Test");

		_c = c;
		
		this.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		Container p = this.getContentPane();
		p.setLayout(new BorderLayout());
		
		_ta = new JTextArea ();
		_ta.setEditable(false);

		JToolBar tb = new JToolBar ();
		JScrollPane sp = new JScrollPane (_ta);
		JToolBar tb2 = new JToolBar ();
		tb.setFloatable(false);
		tb2.setFloatable(false);
		
		WindowListener l = new WindowListener ( this, _c );

		_ip = new JTextField("localhost");
		_port = new JTextField("1380");
		_pseudo = new JTextField("kikOo");
		_connect = new JButton("Connect");
		_connect.addActionListener(l);
		
		_msg = new JTextField("");
		_msg.addKeyListener(l);
		
		tb.add(new JLabel("server ip :"));
		tb.add(_ip);
		tb.add(new JLabel("peer port :"));
		tb.add(_port);
		tb.add(new JLabel("pseudo : "));
		tb.add(_pseudo);
		tb.add(_connect);
		
		tb2.add(_msg);
		
		p.add(BorderLayout.NORTH, tb);
		p.add(BorderLayout.CENTER, sp);
		p.add(BorderLayout.SOUTH, tb2);

		this.setSize(500,350);
		this.setLocation(400,250);
		this.setVisible(true);
	}

	public String displayMsg() {
		String msg = _msg.getText();
		_msg.setText("");
		print(getPseudo()+": " + msg + "\n");
		
		return msg;
	}

	public String getIp() {
		return _ip.getText();
	}

	public short getPort() {
		return ( new Integer(_port.getText()) ).shortValue();
	}

	public String getPseudo() {
		return _pseudo.getText();
	}

	public void print (final String msg) {
		SwingUtilities.invokeLater(new Runnable(){
		    public void run(){
		    	_ta.append (msg);
		    	_ta.setCaretPosition(_ta.getDocument().getLength());
		    }
		});
	}

	public void printError (String err) {
		print (err);
	}

	public void setConnect () {
		_ip.setEnabled(false);
		_port.setEnabled(false);
		_pseudo.setEnabled(false);
		_connect.setEnabled(false);
		
		_msg.setEnabled(true);
	}
}
