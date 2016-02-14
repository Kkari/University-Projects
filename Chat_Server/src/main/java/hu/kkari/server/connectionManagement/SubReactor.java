package hu.kkari.server.connectionManagement;

import java.io.IOException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.SocketChannel;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Queue;


/**
 * @author kkari
 *
 * This is the class responsible for managing selectors. It pipes the work down to the
 * respective handlers on new threads.
 */
public class SubReactor implements Runnable{
	
	private Selector sel;
	private Queue<SocketChannel> reqistrationQueue = new LinkedList<SocketChannel>();
	
	public SubReactor(Selector sel_){
		this.sel = sel_;
	}

	@Override
    public void run() {
        try {
			while (true) {
				sel.select();
				
				while (!reqistrationQueue.isEmpty()) {
					new ConnectionHandler(sel, reqistrationQueue.remove());
				}
				
				Iterator<SelectionKey> it = sel.selectedKeys().iterator();
				while (it.hasNext()) {
					SelectionKey sk = (SelectionKey) it.next();
					it.remove();
					Runnable r = (Runnable) sk.attachment();
					if (r != null)
						r.run();
				}
			}
        	
        }
        catch (IOException ex) {
            ex.printStackTrace();
        }
    }
	
	public void addToRegistrationQueue(SocketChannel newCon) {
		reqistrationQueue.add(newCon);
		sel.wakeup();
	}
}
