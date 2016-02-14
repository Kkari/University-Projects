package hu.kkari.client;

import hu.kkari.chatUtil.messageManagement.Message;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.net.ConnectException;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
import javax.swing.UIManager;


/**
 * @author kkari
 *
 * This is the main class on the client side, it instantiates the Observer and the Networking class,
 * and handles GUI requests, the 3 components communicate through 2 Queues, one for the incoming
 * messages, and one for the outgoing ones.
 */
public class ClientGUI {

	private String      appName     = "JavaChat"; //title 
	private JFrame		chatFrame    = new JFrame(appName); //frame for the chatbox
	private JFrame		baseFrame = new JFrame(appName); //frame just to have a menu
	private	JFrame      loginFrame = new JFrame(appName); //login frame
	private	JButton     sendMessage; 
	private	JTextField  messageBox; //text field for the message
	private	JTextArea   chatBox;	//main text area for the communication
	private	JTextField  usernameChooser; 
	private	JComboBox<Integer> id_number = new JComboBox<Integer>();
	private	String		username;
	private JTextField serverIPBox;
	private JTextField serverPortBox;
    private ClientConnectionHandler connHandler; // the variable to store the networking thread class
    private JLabel errorLabel = new JLabel();
    
    Queue<Message> sendQ = new ConcurrentLinkedQueue<>(); //queue downwards to the networking thread
    Queue<Message> recQ = new ConcurrentLinkedQueue<>(); //queue upwards from the networking thread
    
    public static void main(String[] args) {
    	
    	/* This let's the chatBox to be manipulated by other threads.
    	 * it's necessary, because swing structures are not thread safe,
    	 * and therefore it is impossible to access them from other threads, other than
    	 * they were constructed in, with the exception of making them on the AWT thread,
    	 * and that's exactly what invokeLater does.
    	 */
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                try {
                	//instant fancyator stuff
                    UIManager.setLookAndFeel(UIManager
                            .getSystemLookAndFeelClassName());
                } catch (Exception e) {
                    e.printStackTrace();
                }
                ClientGUI mainGUI = new ClientGUI();
                mainGUI.preDisplay();
            }
        });
    }
    
    /**
     * this display function is only here to force a menu in the application, as it was
     * necessary.
     */
    public void baseDisplay() {
    	
    	JMenuItem mi1 = new JMenuItem("Connect");
		mi1.addActionListener( new ActionListener() {
			
			@Override
			public void actionPerformed(ActionEvent e) {
				baseFrame.setVisible(false);
				preDisplay();
			}
		});
		
		JMenu m1 = new JMenu("Stuff");
		m1.add(mi1);
		
		JMenuBar bar = new JMenuBar();
		bar.add(m1);
		
		baseFrame.setSize(300, 300);
		baseFrame.setJMenuBar(bar);
		baseFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		baseFrame.setVisible(true);
    }
    
    
    /**
     * This display function draws the login frame for the client.
     */
    public void preDisplay() {
        chatFrame.setVisible(false);
        //loginFrame 
        usernameChooser = new JTextField("user");
        serverIPBox = new JTextField("127.0.0.1");
        serverPortBox = new JTextField("9090");
        errorLabel = new JLabel();
        JLabel chooseUsernameLabel = new JLabel("Pick a username:");
        JLabel chooseIdNumber = new JLabel("Choose an id number:");
        JLabel serverIPLabel = new JLabel("Enter Server IP address:");
        JLabel serverPortLabel = new JLabel("Enter server port number:");
        JButton enterServer = new JButton("Enter Chat Server");
        enterServer.addActionListener(new EnterServerButtonListener());
        JPanel prePanel = new JPanel(new GridBagLayout());
        
        usernameChooser.setColumns(20);
        errorLabel.setForeground(Color.RED);
        
        GridBagConstraints userChooserConst = new GridBagConstraints();
        userChooserConst.insets = new Insets(0, 0, 0, 10);
        userChooserConst.anchor = GridBagConstraints.EAST;
        userChooserConst.weightx = 0.5;
        userChooserConst.fill = GridBagConstraints.HORIZONTAL;
        userChooserConst.gridwidth = GridBagConstraints.REMAINDER;
        userChooserConst.gridx = 1;
        userChooserConst.gridy = 0;
        prePanel.add(usernameChooser, userChooserConst);
        
        GridBagConstraints userLabelConst = new GridBagConstraints();
        userLabelConst.anchor = GridBagConstraints.WEST;
        userLabelConst.insets = new Insets(0, 10, 0, 10);
        userLabelConst.gridx = 0;
        userLabelConst.gridy = 0;
        prePanel.add(chooseUsernameLabel, userLabelConst);
        
        GridBagConstraints idLabelConst = new GridBagConstraints();
        idLabelConst.anchor = GridBagConstraints.WEST;
        idLabelConst.insets = new Insets(0, 10, 0, 10);
        idLabelConst.gridx = 0;
        idLabelConst.gridy = 1;
        prePanel.add(chooseIdNumber, idLabelConst);
        
        GridBagConstraints idNumConst = new GridBagConstraints();
        idNumConst.anchor = GridBagConstraints.EAST;
        idNumConst.insets = new Insets(0, 10, 0, 10);
        idNumConst.gridx = 1;
        idNumConst.gridy = 1;
        prePanel.add(id_number, idNumConst);
        
        GridBagConstraints serverIpLabelConst = new GridBagConstraints();
        serverIpLabelConst.anchor = GridBagConstraints.WEST;
        serverIpLabelConst.insets = new Insets(0, 10, 0, 10);
        serverIpLabelConst.gridx = 0;
        serverIpLabelConst.gridy = 2;
        prePanel.add(serverIPLabel, serverIpLabelConst);
        
        GridBagConstraints serverIpBoxConst = new GridBagConstraints();
        serverIpBoxConst.anchor = GridBagConstraints.EAST;
        serverIpBoxConst.insets = new Insets(0, 10, 0, 10);
        serverIpBoxConst.gridx = 1;
        serverIpBoxConst.gridy = 2;
        serverIpBoxConst.fill = GridBagConstraints.HORIZONTAL;
        serverIpBoxConst.gridwidth = GridBagConstraints.REMAINDER;
        prePanel.add(serverIPBox, serverIpBoxConst);
        
        GridBagConstraints serverPortLabelConst = new GridBagConstraints();
        serverPortLabelConst.anchor = GridBagConstraints.WEST;
        serverPortLabelConst.insets = new Insets(0, 10, 0, 10);
        serverPortLabelConst.gridx = 0;
        serverPortLabelConst.gridy = 3;
        prePanel.add(serverPortLabel, serverPortLabelConst);
        
        GridBagConstraints serverPortBoxConst = new GridBagConstraints();
        serverPortBoxConst.anchor = GridBagConstraints.EAST;
        serverPortBoxConst.insets = new Insets(0, 10, 0, 10);
        serverPortBoxConst.gridx = 1;
        serverPortBoxConst.gridy = 3;
        serverPortBoxConst.fill = GridBagConstraints.HORIZONTAL;
        serverPortBoxConst.gridwidth = GridBagConstraints.REMAINDER;
        prePanel.add(serverPortBox, serverPortBoxConst);
        
        GridBagConstraints errorLabelConst = new GridBagConstraints();
        errorLabelConst.anchor = GridBagConstraints.WEST;
        errorLabelConst.insets = new Insets(0, 10, 0, 10);
        errorLabelConst.gridx = 0;
        errorLabelConst.gridy = 4;
        prePanel.add(errorLabel, errorLabelConst);
        
        
        id_number.addItem(1);
        id_number.addItem(2);
        
        loginFrame.add(BorderLayout.CENTER, prePanel);
        loginFrame.add(BorderLayout.SOUTH, enterServer);
        loginFrame.setSize(300, 300);
		loginFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        loginFrame.setVisible(true);

    }

    
    /**
     * This function draws out the main chat window, that one can use to send and receive
     * messages.
     */
    public void display() {
        JPanel mainPanel = new JPanel();
        mainPanel.setLayout(new BorderLayout());

        JPanel southPanel = new JPanel();
        southPanel.setBackground(Color.BLUE);
        southPanel.setLayout(new GridBagLayout());

        SendMessageButtonListener sbl = new SendMessageButtonListener();
        messageBox = new JTextField(30);
        messageBox.requestFocusInWindow();
        messageBox.addActionListener(sbl);
        
        sendMessage = new JButton("Send Message");
        sendMessage.addActionListener(sbl);

        chatBox.setEditable(false);
        chatBox.setFont(new Font("Serif", Font.PLAIN, 15));
        chatBox.setLineWrap(true);

        mainPanel.add(new JScrollPane(chatBox), BorderLayout.CENTER);

        GridBagConstraints left = new GridBagConstraints();
        left.anchor = GridBagConstraints.LINE_START;
        left.fill = GridBagConstraints.HORIZONTAL;
        left.weightx = 512.0D;
        left.weighty = 1.0D;

        GridBagConstraints right = new GridBagConstraints();
        right.insets = new Insets(0, 10, 0, 0);
        right.anchor = GridBagConstraints.LINE_END;
        right.fill = GridBagConstraints.NONE;
        right.weightx = 1.0D;
        right.weighty = 1.0D;

        southPanel.add(messageBox, left);
        southPanel.add(sendMessage, right);

        mainPanel.add(BorderLayout.SOUTH, southPanel);

        chatFrame.add(mainPanel);
        chatFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        chatFrame.setSize(470, 300);
        chatFrame.setVisible(true);
    }
    
    
    /**
     * @author kkari
     * This class handles the message sending, it basically puts the message into the sendQ and the networking thread
     * gets it on the other end.
     */
    class SendMessageButtonListener implements ActionListener {
        public void actionPerformed(ActionEvent event) {
            if (messageBox.getText().length() < 1) {
                // do nothing
            } else if (messageBox.getText().equals(".clear")) {
                chatBox.setText("Cleared all messages\n");
                messageBox.setText("");
            } else if (messageBox.getText().equals(".doge")) {
            	ImageIcon dogeIcon = new ImageIcon("doge.jpeg");
            	JFrame jf = new JFrame();
            	JLabel label = new JLabel("", dogeIcon, JLabel.CENTER);
            	JPanel panel = new JPanel(new BorderLayout());
            	panel.add( label, BorderLayout.CENTER );
            	jf.add(panel);
            	jf.setSize(600, 600);
            	jf.setLocationRelativeTo(null);
            	jf.setVisible(true);
            	messageBox.setText("");
            } else {
                chatBox.append("<" + username + ">:  " + messageBox.getText()
                        + "\n");
                Message tmp = new Message(username,"msg", messageBox.getText());
                messageBox.setText("");
                tmp.setRecipient("all");
                sendQ.add(tmp);
                connHandler.wakeSelector();
            }
            messageBox.requestFocusInWindow();
        }
    }

    class EnterServerButtonListener implements ActionListener {
        public void actionPerformed(ActionEvent event) {
            username = usernameChooser.getText();
            username += (Integer) id_number.getSelectedItem();
            int serverPort = 0;
            
            try {
            	 serverPort = Integer.parseInt(serverPortBox.getText());
            } catch (NumberFormatException nfe) {
            	 errorLabel.setText("invalid Port Number");         	
            	 return;
            }
            
            boolean isIPv4;
            try {
                final InetAddress inet = InetAddress.getByName(serverIPBox.getText());
                isIPv4 = inet.getHostAddress().equals(serverIPBox.getText()) && inet instanceof Inet4Address;
                } catch (final UnknownHostException e) {
                errorLabel.setText("invalid IP address");
                return;
             }
            
            try {
            	chatBox = new JTextArea();
				//--
				try {
					connHandler = new ClientConnectionHandler(recQ, sendQ, serverIPBox.getText(), serverPort);
				} catch (ConnectException ce) {
					errorLabel.setText("Couldn't connect to the server");
					return;
				}
				Thread chThread = new Thread(connHandler);
				chThread.setName("ClientConnHandlerThread");
				chThread.start();
				
				if(!login()) {
					connHandler.shutdownConnection();
					connHandler = null;
					return;
				}
				
			} catch (IOException e ){//| InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			new Thread(new GuiNetworkingInterface(recQ, chatBox)).start();
            loginFrame.setVisible(false);
            display();
        }
       
        
        /**
         * @return returns true if the login was successful
         * 
         * Logs the user into the selected server, it's good here, because for that time it disables the GUI elements on this thread, thus
         * prevents the user to interfere with the process, once it was started.
         * It should have a timer, not to freeze out, when something goes wrong on the network.
         */
        private boolean login() {
        	
        	Message tmp;
        	sendQ.add(new Message(username, "register", "bela"));
        	connHandler.wakeSelector();
        	
        	do {
        		tmp = recQ.poll();
        	} while (tmp == null);
        	
        	if (!tmp.getMessage().equals("registration successful")) {
        		errorLabel.setText("registration not successful");
        		return false;
        	}
        	
        	sendQ.add(new Message(username, "login", "bela"));
        	connHandler.wakeSelector();
        	
         	do {
        		tmp = recQ.poll();
        	} while (tmp == null);
       
        	if (!tmp.getMessage().equals("login successful")) {
        		errorLabel.setText("login not successful");
        		return false;
        	}
        	return true;
        }
    }
}