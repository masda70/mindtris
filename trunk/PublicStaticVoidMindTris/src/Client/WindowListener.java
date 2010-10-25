package Client;

import java.awt.event.*;
import java.io.IOException;
import java.net.UnknownHostException;

public class WindowListener implements ActionListener, KeyListener {
	private Window _w;
	private Client _c;
	
	public WindowListener ( Window w, Client c ) {
		_w = w;
		_c = c;
	}
	
	public void keyPressed(KeyEvent e) {
		int key = e.getKeyCode();
		if (key == KeyEvent.VK_ENTER) {
			try {
				_c.sendMsg(_w.displayMsg());
			} catch (IOException exc) {
				_w.printError("IO Exception : " + exc.getMessage());
			}
		}
	}

	public void keyReleased(KeyEvent e) {		
	}

	public void keyTyped(KeyEvent e) {		
	}

	public void actionPerformed(ActionEvent arg0) {
		try {
			_c.connectToSrv(_w.getIp(), _w.getPort(), _w.getPseudo());		
			_w.setConnect();
		} catch (UnknownHostException e) {
			_w.printError("Unkown Host\n");
		} catch (IOException e) {
			_w.printError("IO Exception : "+e.getMessage()+"\n");
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

}
