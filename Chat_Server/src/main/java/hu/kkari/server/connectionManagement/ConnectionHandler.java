package hu.kkari.server.connectionManagement;

import hu.kkari.chatUtil.messageManagement.ContinuousJsonParser;
import hu.kkari.chatUtil.messageManagement.ContinuousMessageParser;
import hu.kkari.chatUtil.messageManagement.Message;
import hu.kkari.server.clientManagement.ClientRecorder;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.SocketChannel;
import java.nio.charset.Charset;
import java.util.Set;

import javax.naming.NameNotFoundException;


/**
 * @author kkari
 * 
 * This is the main class handling the connection of each client it's bound to a ClientEntry and a SocketChannel
 */
public class ConnectionHandler implements Handler, Runnable{
	
    private final SocketChannel channel;
    private final SelectionKey selKey;

    private static final int READ_BUF_SIZE = 1024;
    private  static final int WRiTE_BUF_SIZE = 1024;
    private  ByteBuffer readBuf = ByteBuffer.allocate(READ_BUF_SIZE);
    private  ByteBuffer writeBuf = ByteBuffer.allocate(WRiTE_BUF_SIZE);
    private ContinuousMessageParser cmp = new ContinuousJsonParser();
    private boolean closeConnection = false;
    private String name = null;
   // Serializer sr = new JSONSerializer();
    
	/**
	 * @param sel The selector that needs to register the new SocketChannel
	 * @param sc SocketChannel bound to this handler
	 * @throws IOException propagates the exception of write and read
	 */
	public ConnectionHandler(Selector sel, SocketChannel sc) throws IOException {
        channel = sc;
        channel.configureBlocking(false);

        // Register the socket channel with interest-set set to READ operation
        selKey = channel.register(sel, SelectionKey.OP_READ);
        selKey.attach(this);
        selKey.interestOps(SelectionKey.OP_READ);
        sel.wakeup();
	}
	
	public void run() {
        try {
            if (selKey.isReadable())
                read();
            else if (selKey.isWritable()) {
            	synchronized (this) {
            		this.notifyAll();
            	}
    			write();
            }
        }
        catch (IOException ex) {
            ex.printStackTrace();
        }
    }

	private void process() {
        byte[] bytes;
        
        synchronized (readBuf) {
        	readBuf.flip();
        	bytes = new byte[readBuf.remaining()];
        	readBuf.get(bytes, 0, bytes.length);
        	readBuf.clear();
        	System.out.println("process(): " + new String(bytes, Charset.forName("ISO-8859-1")));
        }

        Message m = cmp.evaluateBuffer(bytes);
        if(m != null) {
        	System.out.println("Got JSON " + m.getSender() + " " + m.getRecipient() + " " + m.getMessage());
        	
        	switch (m.getType()) {
			case "register":
				if(ClientRecorder.getInstance().register(m.getSender(), m.getMessage()))
					this.sendString(new Message("server","system", "registration successful"));
				else {
					this.sendString(new Message("server","system", "registration not successful"));
					closeConnection = true;
				}
				break;
			case "login":
				if(ClientRecorder.getInstance().login(m.getSender(), m.getRecipient(), this)) {
					this.name = m.getSender();
					this.sendString(new Message("server","system", "login successful"));
				}
				else {
					this.sendString(new Message("server","system", "login not successful"));
					closeConnection = true;
				}
				break;
			case "msg":
				try {
					if(!m.getRecipient().equals("all")) {
						Handler tmp = ClientRecorder.getInstance().getHandler(m.getRecipient());
						tmp.sendString(m);
					}
					else {
						Set<String> clients = ClientRecorder.getInstance().getAllLoggedInClients();
						for(String name : clients) {
							if(!m.getSender().equals(name)) {
								System.out.println("Sending to " + name);
								ClientRecorder.getInstance().getHandler(name).sendString(m);
							}
						}
					}
				} catch (NameNotFoundException e) {
					this.sendString(new Message("error", "recipient not found"));
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				break;
			default:
				this.sendString(new Message("error", "invalid type"));
				break;
			}
        }
        if(!closeConnection) selKey.interestOps(SelectionKey.OP_READ); // enabling part of my horrible hack
    	selKey.selector().wakeup();
    }
    
    /**
     * @throws IOException
     * 
     * This function is still executed in the selector manager thread, after reading from the channel buffer it passes
     * the work off to a worker thread, or signals by numBytes == -1 that the channel was closed by the remote host.
     */
    private void read() throws IOException {
        int numBytes;

        try {
            numBytes = channel.read(readBuf);
            System.out.println("read(): #bytes read into 'readBuf' buffer = " + numBytes);
            selKey.interestOps(SelectionKey.OP_CONNECT); // horrible hack to disable the selector while working on it's data
         // selKey.selector().wakeup();
            if (numBytes == -1) {
            	if(name != null) {
            		ClientRecorder.getInstance().logout(name);
            	}
                selKey.cancel();
                channel.close();
                System.out.println("read(): client connection might have been dropped!");
            }
            else {
                MainReactor.workerPool.execute(new Runnable() {
                    public void run() {
                        process();
                    }
                });
            }
        }
        catch (IOException ex) {
            ex.printStackTrace();
            return;
        }
    }

    private void write() throws IOException {
        int numBytes = 0;

        try {
            numBytes = channel.write(writeBuf);
            System.out.println("write(): #bytes read from 'writeBuf' buffer = " + numBytes + " to " + this.hashCode());
            
            if(closeConnection) {
            	selKey.cancel();
            	channel.close();
            	return;
            }
            
            if(numBytes > 0) {
            //	readBuf.clear();
            	writeBuf.clear();
            	// Set the key's interest-set back to READ operation
            	selKey.interestOps(SelectionKey.OP_READ);
            	selKey.selector().wakeup();
            }
        }
        catch (IOException ex) {
            ex.printStackTrace();
        }
    }

	@Override
	public void sendString(Message msg){

		writeBuf = ByteBuffer.wrap(msg.Serialize());
		selKey.interestOps(SelectionKey.OP_WRITE);
		System.out.println("lol");
		synchronized (this) {
			selKey.selector().wakeup();
			try {
				this.wait();
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
	
	public ContinuousMessageParser getMessageProcessor() {
		return cmp;
	}
}
