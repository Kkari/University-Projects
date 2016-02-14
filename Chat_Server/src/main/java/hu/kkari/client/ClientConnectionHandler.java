package hu.kkari.client;

import hu.kkari.chatUtil.messageManagement.ContinuousJsonParser;
import hu.kkari.chatUtil.messageManagement.ContinuousMessageParser;
import hu.kkari.chatUtil.messageManagement.Message;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.SocketChannel;
import java.util.Queue;


/**
 * @author kkari
 * 
 * This class handles all the network communication on the client side. 
 * It uses the same non-blocking networking IO as the server side.
 *
 */
public class ClientConnectionHandler implements Runnable {
	private Queue<Message> sendQ; //Queue from the GUI
	private Queue<Message> recQ;  //Queue to the interface
	private ContinuousMessageParser cmp = new ContinuousJsonParser();
    static final int READ_BUF_SIZE = 1024;
    static final int WRiTE_BUF_SIZE = 1024;
    private ByteBuffer readBuf = ByteBuffer.allocate(READ_BUF_SIZE);
	private Selector sel = Selector.open();
	private SocketChannel channel;
	private boolean terminate = false;
	
	public ClientConnectionHandler(Queue<Message> recQ_, Queue<Message> sendQ_, String ip, int port) throws UnknownHostException, IOException {
		sendQ = sendQ_;
		recQ = recQ_;
		
        channel = SocketChannel.open();
        channel.configureBlocking(true);
        
        channel.connect(new InetSocketAddress(ip, port));
        channel.configureBlocking(false);
        channel.register(sel, SelectionKey.OP_READ);
	}
	
	public void wakeSelector() {
		sel.wakeup();
	}
	
	public void shutdownConnection() throws IOException {
		terminate = true;
		sel.wakeup();
	}
	
	@Override
	public void run() {
		try {
			while(true) {
				
				if(terminate) {
					sel.close();
					channel.close();
					return;
				} else {
					sel.select();
				}
				
				channel.read(readBuf);
				
				if(readBuf.hasRemaining()) {
			        byte[] bytes;
			        readBuf.flip();
			        bytes = new byte[readBuf.remaining()];
			        readBuf.get(bytes, 0, bytes.length);
					readBuf.clear();
					
					Message mr = cmp.evaluateBuffer(bytes);
					if (mr != null) {
						recQ.add(mr);
						synchronized (recQ) {
					//		System.out.println("lol");
							recQ.notify();
						}
					}
				}
				
				Message ms = sendQ.poll();
				if( ms != null)
					channel.write(ByteBuffer.wrap(ms.Serialize()));
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
}
