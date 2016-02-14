package hu.kkari.server.connectionManagement;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * @author kkari
 * The main reactor class of the server, this is, where connections get accepted,
 * and where they get assigned to one of the SubReactors, only SubReactors have a
 * thread, and the sockets assigned will be managed through Selectors, in order to
 * keep a good balance between multiplexed and thread based communication. The number
 * of SubReactors scale based on the number of CPU cores. And these SubReactors assign
 * work to Threads in the shared workerPool, when they have incoming requests.
 */
public class MainReactor implements Runnable {

    private List<Selector> selectors = new ArrayList<Selector>();
    private List<SubReactor> subReactors = new ArrayList<SubReactor>();
    
    private volatile boolean running = true; //TODO implement shutdown mechanism
    private final ServerSocketChannel serverChannel;
    private int magicBalancer;
    static ExecutorService workerPool, selectorPool;
	
	public MainReactor(int port) throws IOException {
		
		int numproc = Runtime.getRuntime().availableProcessors();
	    magicBalancer = (int) Math.ceil(numproc/2.0);
		workerPool = Executors.newFixedThreadPool(magicBalancer);
		selectorPool = Executors.newFixedThreadPool(magicBalancer);
		
		for(int i = 0; i < magicBalancer; i++) {
			selectors.add(Selector.open());
			subReactors.add(new SubReactor(selectors.get(i)));
			selectorPool.execute(subReactors.get(i));
		}
		
		serverChannel = ServerSocketChannel.open();
		serverChannel.socket().bind(new InetSocketAddress(port));
		System.out.println("The server has started");

	}

	public void run() {
		int curr = 0;
			while (true) {
				try {
					SocketChannel channel = serverChannel.accept();
					System.out.println("Client connected from " + channel.getRemoteAddress());
					if (channel != null) {
						subReactors.get((curr++) % magicBalancer)
								.addToRegistrationQueue(channel);
					}
				} catch (IOException ex) {
				}
				if(running == false) {
					try {
						serverChannel.close();
						selectorPool.shutdown();
						workerPool.shutdown();
						System.exit(0);
					} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			} 
	}
	
	public void shutdown() throws IOException {
		serverChannel.close();
	}
}
