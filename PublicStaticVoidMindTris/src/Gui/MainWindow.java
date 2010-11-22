package Gui;

import Client.*;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.WindowEvent;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.UnknownHostException;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import javax.swing.*;
import javax.swing.text.JTextComponent;

import Server.Server;
import Util.Channel;
import Util.Lobby;
import Util.Peer;

public class MainWindow extends JFrame {
	private static final long serialVersionUID = 1L;
	private Client _c;

	private TxtField _error = new TxtField();
	private JComponent _text = new Lbl("PublicStaticVoidMindTris");
	private JComponent _top,
					   _center;
	
	public MainWindow ( Client c ) {
		super("MindTris");

		_c = c;
		
		this.addWindowListener(new java.awt.event.WindowAdapter() {
			public void windowClosing(WindowEvent ev) {
				System.exit(0);
		    }
		});

		this.setBackground(new Color(30, 30, 30));
		this.setSize(800,600);
		
		paintMenu();
		
		setLocation(50, 50);
		setVisible(true);
	}
	
	////////// PAINT ///////////
	
	public void paintMenu () {
		Container p = this.getContentPane();
		
		final TxtField srvIp = new TxtField("localhost"),
					   port = new TxtField((new Short(_c._port)).toString());

		ActionListener connectListener = new ActionListener() {
			public void actionPerformed(ActionEvent ev) {
				try {
					_c._port = new Short(port.getText());
					_c.connectToSrv(srvIp.getText());
					paintLogin();
				} catch (UnknownHostException e) {
					printError("Unkown Host");
				} catch (IOException e) {
					printError("IO Exception : "+e.getMessage());
				}
			}
		};
		
		ActionListener createListener = new ActionListener() {
			public void actionPerformed(ActionEvent ev) {
				print("launch server");
				
				final Server srv = new Server();
				srv.start();
				
			/*	SwingUtilities.invokeLater(new Runnable() {
					public void run () {
						try {
							synchronized (srv) {
								srv.wait();
							}
							_c.connectToSrv("localhost");
							paintLogin();
						} catch (IOException e) {
							printError("IO Exception : "+e.getMessage());
						} catch (InterruptedException e) {
							e.printStackTrace();
						}
					}
				}); */
			}
		};
		
		
		srvIp.addActionListener(connectListener);
		port.addActionListener(connectListener);
		Btn connect = new Btn("connect");
		connect.addActionListener(connectListener);
		Btn createServer = new Btn("Create a server");
		createServer.addActionListener(createListener);
		
		p.setLayout(new BorderLayout());
		_top = new JToolBar();
		((JToolBar) _top).setFloatable(false);
		
		_top.add(new Lbl("server ip :"));
		_top.add(srvIp);
		_top.add(new Lbl("peer port :"));
		_top.add(port);
		_top.add(connect);
		_top.add(createServer);
		_top.setBorder(BorderFactory.createCompoundBorder(
				BorderFactory.createMatteBorder(0, 0, 1, 0, Color.GRAY),
				BorderFactory.createEmptyBorder(4, 10, 5, 10)
		));
		
		_center = new JPanel(new GridBagLayout());
		_center.add(_text);
		
		_error.setEditable(false);
		_error.setBorder(BorderFactory.createCompoundBorder(
				BorderFactory.createMatteBorder(1, 0, 0, 0, Color.GRAY),
				BorderFactory.createEmptyBorder(3, 10, 5, 10)
		));
		
		p.add(BorderLayout.NORTH, _top);
		p.add(BorderLayout.CENTER, _center);
		p.add(BorderLayout.SOUTH, _error);
	}

	public void paintLogin () {
		final TxtField usr = new TxtField();
		final PwdField pwd = new PwdField();
		
		ActionListener loginListener = new ActionListener () {
			public void actionPerformed(ActionEvent ev) {
				try {
					byte [] usrBuf = usr.getText().getBytes(Channel.ENCODING);
					byte [] pwdBuf = ( new String(pwd.getPassword()) ).getBytes(Channel.ENCODING);
					_c.login(usrBuf, pwdBuf);
				} catch (IOException e) {
					printError("IO Exception : "+e.getMessage());
				}
			}
		};
		
		usr.addActionListener(loginListener);
		pwd.addActionListener(loginListener);
		
		Btn login = new Btn("login");
		login.addActionListener(loginListener);
		
		final Btn createUser = new Btn("create user...");
		
		ActionListener createListener = new ActionListener () {
			public void actionPerformed(ActionEvent arg0) {
				paintCreateUsr();
			}
		};
		
		createUser.addActionListener(createListener);
		
		_top.removeAll();
		_top.add(new Lbl("user :"));
		_top.add(usr);
		_top.add(new Lbl("password :"));
		_top.add(pwd);
		_top.add(login);
		_top.add(createUser);
		
		repaint();
	}

	public void paintCreateUsr () {
		final TxtField usr = new TxtField(15),
					   displayName = new TxtField(15),
					   email = new TxtField(15);
		final PwdField pwd = new PwdField(15);
		Btn create = new Btn("Create User");
		
		ActionListener listener = new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				try {
					_c.createUser(usr.getText(), displayName.getText(), email.getText(), pwd.getPassword());
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		};

		create.addActionListener(listener);
		
		String[] lbls = new String[] {"User name :","Display name :", "Password :", "Email :"};
		JTextField[] fields = new JTextField[]{usr, displayName, pwd, email};

		GridBagConstraints c = new GridBagConstraints();
		_center.removeAll();

		c.gridwidth = GridBagConstraints.REMAINDER;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = new Insets(20,0,10,0);
		_center.add(new Lbl("Please fill the form"), c);
		
		c.insets = new Insets(0,10,5,0);

		for( int i=0; i<lbls.length; i++ ) {
			fields[i].addActionListener(listener);
			
			c.gridwidth = GridBagConstraints.RELATIVE;
			c.anchor = GridBagConstraints.EAST;
			_center.add(new Lbl(lbls[i]),c);
			
			c.gridwidth = GridBagConstraints.REMAINDER;
			c.anchor = GridBagConstraints.WEST;
			_center.add(fields[i], c);
		}
		
		_center.add(create, c);

		_center.repaint();
		_center.revalidate();
	}

	public void paintCreateLobby () {
		final TxtField name = new TxtField(15);
		final PwdField pwd = new PwdField(15);
		final IntSpinner maxPlayers = new IntSpinner(10, 1, 10);
		Btn create = new Btn("Create Lobby");
		
		ActionListener listener = new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				try {
					_c.createLobby(name.getText(), pwd.getPassword(), maxPlayers.getNb());
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		};

		name.addActionListener(listener);
		pwd.addActionListener(listener);
		create.addActionListener(listener);
		
		String[] lbls = new String[] {"Lobby name :", "Password (or empty) :", "Max player allowed :"};
		JComponent[] fields = new JComponent[]{name, pwd, maxPlayers};

		GridBagConstraints c = new GridBagConstraints();
		_center.removeAll();

		c.gridwidth = GridBagConstraints.REMAINDER;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = new Insets(20,0,10,0);
		_center.add(new Lbl("Lobby parameters"), c);
		
		c.insets = new Insets(0,10,5,0);

		for( int i=0; i<lbls.length; i++ ) {
			c.gridwidth = GridBagConstraints.RELATIVE;
			c.anchor = GridBagConstraints.EAST;
			_center.add(new Lbl(lbls[i]),c);
			
			c.gridwidth = GridBagConstraints.REMAINDER;
			c.anchor = GridBagConstraints.WEST;
			_center.add(fields[i], c);
		}
		
		_center.add(create, c);

		_center.repaint();
		_center.revalidate();
	}
	
	public void paintLobbyList ( List<Lobby> list ) {
		Btn create = new Btn("create lobby...");
		create.addActionListener( new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				paintCreateLobby();
			}
		});
		
		final Lbl pwdLbl = new Lbl("Please enter password :");
		final PwdField pwdField = new PwdField();
		final Btn pwdBtn = new Btn("join");
		
		_top.removeAll();
		_top.add(new Lbl("Server lobby list"));
		_top.repaint();
		
		_center.removeAll();

		GridBagConstraints c = new GridBagConstraints();
		c.anchor = GridBagConstraints.CENTER;
		c.insets = new Insets(0,15,10,15);
		
		if( list.size() > 0 ) {
			c.gridwidth = GridBagConstraints.RELATIVE;
			_center.add(new Lbl("Lobby"), c);
			_center.add(new Lbl("creator"), c);
			_center.add(new Lbl("players"), c);
			c.gridwidth = GridBagConstraints.REMAINDER;
			_center.add(new Lbl("pwd required"), c);
	
			c.anchor = GridBagConstraints.WEST;
			c.insets = new Insets(0,15,5,15);
			for( final Lobby l : list ) {
				Lbl name = new Lbl(new String(l._name));
				Lbl creator = new Lbl(new String(l._creator));
				Lbl players = new Lbl( Integer.toString((int)l._nbPlayers)
									+ "/" + Integer.toString((int)l._maxPlayers));
				Lbl pwd = new Lbl(l.pwdRequired() ? "yes" : "");
					
				name.addMouseListener(new MouseListener () {
					public void mouseClicked(MouseEvent arg0) {
						if( l.pwdRequired() ) {
							ActionListener join = new ActionListener() {
								public void actionPerformed(ActionEvent arg0) {
									try {
										byte[] pwd = (new String( pwdField.getPassword())).getBytes(Channel.ENCODING);
										_c.joinLobby(l._id, l.getName(), l.getCreator(), pwd);
									} catch ( IOException e ) {
										// TODO Auto-generated catch block
										e.printStackTrace();
									}
								}
							};
							pwdBtn.addActionListener(join);
							pwdField.addActionListener(join);
							
							_top.removeAll();
							_top.add(pwdLbl);
							_top.add(pwdField);
							_top.add(pwdBtn);
							_top.repaint();
						} else {
							try {
								_c.joinLobby(l._id, l.getName(), l.getCreator(), new byte[]{0x00});
							} catch (IOException e) {
								// TODO Auto-generated catch block
								e.printStackTrace();
							}
						}
					}
	
					public void mouseEntered(MouseEvent arg0) {}
					public void mouseExited(MouseEvent arg0) {}
					public void mousePressed(MouseEvent arg0) {}
					public void mouseReleased(MouseEvent arg0) {}
				});
				name.setForeground(Color.LIGHT_GRAY);
				name.setCursor(new Cursor(Cursor.HAND_CURSOR));
				
				c.gridwidth = GridBagConstraints.RELATIVE;
				_center.add(name, c);
				_center.add(creator, c);
				_center.add(players, c);
				c.gridwidth = GridBagConstraints.REMAINDER;
				_center.add(pwd, c);
			}
		} else {
			c.gridwidth = GridBagConstraints.REMAINDER;
			_center.add(new Lbl("No lobby on this server"), c);
		}
		c.anchor = GridBagConstraints.CENTER;
		c.insets = new Insets(10,0,10,0);
		_center.add(create, c);
		
		_center.repaint();
		_center.revalidate();
	}

	public void printLobby ( Lobby l, final String displayName ) {
		Lbl name = new Lbl(l.getName() + " (created by "+l.getCreator()+")");
		
		JPanel connected = new JPanel();
		Lbl connectedLbl = new Lbl("peers :");
		connected.add(connectedLbl);
		
		connected.add(new Lbl(displayName + ", "));
		
		for( Entry<Integer, Peer> o : l._peers ) {
			Peer p = o.getValue();
			connected.add(new Lbl(p.getName()+", "));
		}
		
		_text = new TxtArea();
		final TxtField msg = new TxtField();
		
		((TxtArea) _text).setEditable(false);
		msg.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				_c.sendChatMsg(msg.getText());
				((TxtArea) _text).append(displayName + ": " + msg.getText() + "\n");
				msg.setText("");
			}
		});
		
		_top.removeAll();
		_top.add(name);
		_top.repaint();
		
		_center.removeAll();
		_center.setLayout(new BorderLayout());

		_center.add(connected, BorderLayout.NORTH);
		_center.add(_text, BorderLayout.CENTER);
		_center.add(msg, BorderLayout.SOUTH);
		
		_center.repaint();
		_center.revalidate();
	}

	/////////// PRINT ///////////
	
	public void print (String txt) {
		_error.setForeground(Color.WHITE);
		_error.setText(txt);
	}
	
	public void printChatMsg ( Peer p, String msg ) {
		String txt = new String( p.getName() + ": " + msg + "\n");
		_text.setForeground(Color.WHITE);
		((TxtArea) _text).append(txt);
	}

	public void printNewPeer ( Peer p ) {
		_text.setForeground(Color.LIGHT_GRAY);
		((TxtArea) _text).append(p.getName() + " has join the lobby\n");
	}
	
	public void printError (String txt) {
		_error.setForeground(Color.RED);
		_error.setText(txt);
	}
	
	public void printCenter(String txt) {
		((Lbl) _text).setText(txt);
		_center.repaint();		
	}
}
