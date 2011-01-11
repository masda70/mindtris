package Gui;

import Client.*;
import Server.Server;
import Util.*;
import Encodings.*;
import Game.*;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.WindowEvent;
import java.io.IOException;
import java.net.UnknownHostException;
import java.util.List;
import java.util.Map.Entry;

import javax.swing.*;


public class MainWindow extends JFrame {
	////// STATIC FIELDS //////
	private static final long serialVersionUID = 1L;
	private static final Color bg = new Color(30, 30, 30); // kikoo style
	
	////// FIELDS //////
	private Client		_c;
	private TxtField	_error = new TxtField();
	private JComponent	_text = new JLabel(new ImageIcon("logo.png"));
	private JComponent	_top,
					  	_center;
	private GameLeft	_left;
	private GameRight	_right;
	private TxtField	_chatBar;
	private Board		_board;
	private KeyListener _keyListener;
	private boolean		_inGame;
	
	////// CONSTRUCTORS //////
	public MainWindow ( Client c ) {
		super("MindTris");

		_c = c;
		_keyListener = new KeyListener() {
			public void keyTyped(KeyEvent ev) {}
			public void keyReleased(KeyEvent ev) {}
			public void keyPressed(KeyEvent ev) {
				try {
					switch( ev.getKeyCode() ) {
					case KeyEvent.VK_LEFT:
						_c.getGame().leftMove();
						break;
					case KeyEvent.VK_UP:
						_c.getGame().rotate();
						break;
					case KeyEvent.VK_RIGHT:
						_c.getGame().rightMove();
						break;
					case KeyEvent.VK_DOWN:
						_c.getGame().softDrop();
						break;
					case KeyEvent.VK_SPACE:
						_c.getGame().hardDrop();
						break;
					default:
						_chatBar.requestFocusInWindow();
						_chatBar.setText(KeyEvent.getKeyText(ev.getKeyCode()).toLowerCase());
						_chatBar.setCaretPosition(1);
					}
					
					_board.repaint();
				} catch ( IOException e ) {
					e.printStackTrace();
				}
			}
		};
		
		this.addWindowListener(new java.awt.event.WindowAdapter() {
			public void windowClosing(WindowEvent ev) {
				System.exit(0);
		    }
		});
		
		this.getContentPane().setBackground(bg);
		this.setSize(800,600);
		
		paintMenu();
		
		setLocation(50, 50);
		setVisible(true);
	}
	
	////// PAINT //////
	public void paintMenu () {
		Container p = this.getContentPane();
		
		final TxtField srvIp = new TxtField("localhost"),
					   port = new TxtField((new Integer(_c._port)).toString());

		BtnListener connectListener = new BtnListener() {
			public void action() {
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
		
		BtnListener createListener = new BtnListener() {
			public void action() {
				print("launch server");
				
				final Server srv = new Server();
				srv.start();
			}
		};
		
		srvIp.addActionListener(connectListener);
		port.addActionListener(connectListener);
		Btn connect = new Btn("connect");
		connect.addBtnListener(connectListener);
		Btn createServer = new Btn("Create a server");
		createServer.addBtnListener(createListener);
		
		p.setLayout(new BorderLayout());
		_top = new JToolBar();
		_top.setBackground(bg);
		((JToolBar) _top).setFloatable(false);
		
		_top.add(new Lbl("server ip : "));
		_top.add(srvIp);
		((JToolBar) _top).addSeparator();
		_top.add(new Lbl("peer port : "));
		_top.add(port);
		((JToolBar) _top).addSeparator();
		_top.add(connect);
		((JToolBar) _top).addSeparator();
		_top.add(createServer);
		_top.setBorder(BorderFactory.createCompoundBorder(
				BorderFactory.createMatteBorder(0, 0, 1, 0, Color.GRAY),
				BorderFactory.createEmptyBorder(4, 10, 5, 10)
		));
		
		_center = new JPanel(new GridBagLayout());
		_center.setBackground(bg);
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
		
		BtnListener loginListener = new BtnListener () {
			public void action() {
				try {
					UString u = usr.getUTxt();
					AString p = pwd.getPwd();
					if( u != null && p != null ) _c.login(u, p);
				} catch (IOException e) {
					printError("IO Exception : "+e.getMessage());
				}
			}
		};
		
		usr.addActionListener(loginListener);
		pwd.addActionListener(loginListener);
		
		Btn login = new Btn("login");
		login.addBtnListener(loginListener);
		
		final Btn createUser = new Btn("create user...");
		
		BtnListener createListener = new BtnListener () {
			public void action() {
				paintCreateUsr();
			}
		};
		
		createUser.addBtnListener(createListener);
		
		_top.removeAll();
		_top.add(new Lbl("user : "));
		_top.add(usr);
		((JToolBar) _top).addSeparator();
		_top.add(new Lbl("password : "));
		_top.add(pwd);
		((JToolBar) _top).addSeparator();
		_top.add(login);
		((JToolBar) _top).addSeparator();
		_top.add(createUser);
		
		usr.requestFocusInWindow();
		repaint();
	}

	public void paintCreateUsr () {
		final TxtField usr = new TxtField(15),
					   displayName = new TxtField(15),
					   email = new TxtField(15);
		final PwdField pwd = new PwdField(15);
		Btn create = new Btn("Create User");
		
		BtnListener listener = new BtnListener() {
			public void action() {
				try {
					_c.createUser(usr.getUTxt(), displayName.getUTxt(), email.getATxt(), pwd.getPwd());
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		};

		create.addBtnListener(listener);
		
		String[] lbls = new String[] {"User name :","Display name :", "Password :", "Email :"};
		JTextField[] fields = new JTextField[]{usr, displayName, pwd, email};

		GridBagConstraints c = new GridBagConstraints();
		_center.removeAll();

		c.gridwidth = GridBagConstraints.REMAINDER;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = new Insets(20,0,20,0);
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

		fields[0].requestFocusInWindow();
		_center.repaint();
		_center.revalidate();
	}

	public void paintCreateLobby () {
		final TxtField name = new TxtField(15);
		final PwdField pwd = new PwdField(15);
		final IntSpinner maxPlayers = new IntSpinner(10, 1, 10);
		Btn create = new Btn("Create Lobby");
		
		BtnListener listener = new BtnListener() {
			public void action() {
				try {
					_c.createLobby(name.getUTxt(), pwd.getPwd(), maxPlayers.getNb());
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		};

		name.addActionListener(listener);
		pwd.addActionListener(listener);
		create.addBtnListener(listener);
		
		String[] lbls = new String[] {"Lobby name :", "Password (or empty) :", "Max player allowed :"};
		JComponent[] fields = new JComponent[]{name, pwd, maxPlayers};

		GridBagConstraints c = new GridBagConstraints();
		_center.removeAll();

		c.gridwidth = GridBagConstraints.REMAINDER;
		c.anchor = GridBagConstraints.CENTER;
		c.insets = new Insets(20,0,20,0);
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
		fields[0].requestFocusInWindow();
		
		_center.repaint();
		_center.revalidate();
	}
	
	public void paintLobbyList ( List<Lobby> list ) {
		Btn create = new Btn("create lobby...");
		create.addBtnListener( new BtnListener() {
			public void action() {
				paintCreateLobby();
			}
		});
		Btn refresh = new Btn("refresh");
		refresh.addBtnListener(new BtnListener() {
			public void action(){
				try {
					_c.getLobbyList();
				} catch (IOException e) {
					printError(e.getMessage());
				}
			}
		});
		
		final Lbl pwdLbl = new Lbl("Please enter password : ");
		final PwdField pwdField = new PwdField();
		final Btn pwdBtn = new Btn("join");
		
		_top.removeAll();
		_top.add(new Lbl("Server lobby list"));
		((JToolBar) _top).addSeparator();
		_top.add(refresh);
		_top.repaint();
		
		_center.removeAll();
		_center.setLayout(new GridBagLayout());

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
				Lbl name = new Lbl(l._name.v());
				Lbl creator = new Lbl(l.getCreatorName());
				Lbl players = new Lbl( Integer.toString(l._nbPlayers)
									+ "/" + Integer.toString(l._maxPlayers));
				Lbl pwd = new Lbl(l.pwdRequired() ? "yes" : "");
					
				name.addMouseListener(new MouseListener () {
					public void mouseClicked(MouseEvent arg0) {
						if( l.pwdRequired() ) {
							ActionListener join = new ActionListener() {
								public void actionPerformed(ActionEvent arg0) {
									try {
										_c.joinLobby(l._id, pwdField.getPwd());
									} catch ( IOException e ) {
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
								_c.joinLobby(l._id, null);
							} catch (IOException e) {
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
		create.requestFocusInWindow();
		
		_center.repaint();
		_center.revalidate();
	}

	public void printLobby ( Lobby l, final UString displayName, boolean isCreator ) {
		_inGame = false;
		
		Btn quit = new Btn("Quit");
		quit.addBtnListener(new BtnListener() {
			public void action() {
				try {
					removeKeyListener(_keyListener);
					_c.quitLobby();
				} catch ( IOException e ) {
					printError(e.getMessage());
				}
			}
		});
		Lbl name = new Lbl(l._name.v() + " (created by "+l.getCreatorName()+")");
		Btn start = null;
		
		if( isCreator ) {
			start = new Btn("Start");
			start.addBtnListener(new BtnListener() {
				public void action() {
					try {
						_c.startGame();
					} catch (IOException e) {
						printError(e.getMessage());
					}
				}
			});
		}
		
		JPanel connected = new JPanel();
		connected.setBackground(bg);
		connected.add(new Lbl("peers :"));
		
		for( Entry<Integer, Peer> o : l._peers ) {
			Peer p = o.getValue();
			connected.add(new Lbl(p._displayName.v() + ", "));
		}
		
		_text = new TxtArea();
		_chatBar = new TxtField();
		
		((TxtArea) _text).setEditable(false);
		_chatBar.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ev) {
				try {
					_c.sendChatMsg(_chatBar.getUTxt());
					((TxtArea) _text).append(displayName.v() + ": " + _chatBar.getText() + "\n");
					_chatBar.setText("");
					if( _inGame ) requestFocusInWindow();
				} catch ( IOException e ) {
					printError(e.getMessage());
				}
			}
		});
		_chatBar.addKeyListener(new KeyListener() {
			public void keyTyped(KeyEvent e) {}
			public void keyReleased(KeyEvent e) {}
			public void keyPressed(KeyEvent e) {
				if( e.getKeyCode() == KeyEvent.VK_ESCAPE) {
					_chatBar.setText("");
					requestFocusInWindow();
				}
			}
		});
		
		_top.removeAll();
		_top.add(quit);
		((JToolBar) _top).addSeparator();
		_top.add(name);
		if( isCreator ) {
			((JToolBar) _top).addSeparator();
			_top.add(start);
		}
		_top.repaint();
		
		_center.removeAll();
		_center.setLayout(new BorderLayout());

		_center.add(connected, BorderLayout.NORTH);
		_center.add(new JScrollPane(_text), BorderLayout.CENTER);
		_center.add(_chatBar, BorderLayout.SOUTH);
		
		if( isCreator ) start.requestFocusInWindow();
		_center.repaint();
		_center.revalidate();
	}

	public void loadGame () {
		Game g = _c.getGame();
		
		_left = new GameLeft(g.nextPieces());
		_board = new Board(g);
		_right = new GameRight(_c.getPeerGames(), _c.getPeers(), _text, _chatBar);
		
		_center.removeAll();
		_center.setLayout(null);
		
		int w=_center.getWidth(),
			h=2*Board.BORDER_SZ + ((_center.getHeight()-2*Board.BORDER_SZ)/Game.H) * Game.H,
			w2=2*Board.BORDER_SZ + ((h-2*Board.BORDER_SZ)/Game.H) * Game.W;
		
		_left.setBounds(0, 0, 100, h);
		_center.add(_left);
		_board.setBounds(100, 0, w2, h);
		_center.add(_board);
		_right.setBounds(100+w2, 0, w-80-w2, h);
		_center.add(_right);
		
		_center.repaint();
		_center.revalidate();
	}
	
	public void beginGame () {
		_inGame = true;
		addKeyListener(_keyListener);

		requestFocusInWindow();
		upBoard();
	}

	public void gameOver() {
		_inGame = false;
		_board.drawGameOver();
		
		removeKeyListener(_keyListener);
	}
	
	////// PRINT //////	
	public void print (String txt) {
		_error.setForeground(Color.WHITE);
		_error.setText(txt);
	}
	
	public void printChatMsg ( Peer p, UString txt ) {
		String s = new String( p._displayName.v() + ": " + txt.v() + "\n");
		((TxtArea) _text).append(s);
	}

	public void printNewPeer ( Peer p ) {
		((TxtArea) _text).append(p._displayName.v() + " has join the lobby\n");
	}

	public void printPeerLeaving ( Peer p ) {
		((TxtArea) _text).append(p._displayName.v() + " has leave the lobby\n");
	}
	
	public void printError (String txt) {
		_error.setForeground(Color.RED);
		_error.setText(txt);
	}
	
	public void printCenter(String txt) {
		if( _text instanceof Lbl ) ((Lbl) _text).setText(txt);
		else _text = new Lbl(txt);
		
		_center.repaint();		
	}

	public void upBoard() {
		_board.repaint();
	}

	public void upNextPieces() {
		_left.repaint();
	}
	
	public void upPeerBoards ( int peerId ) {
		_right.upPeerBoards(peerId);
	}

	public void addScore ( int nbLines ) {
		_left.addScore(nbLines);
	}
}
