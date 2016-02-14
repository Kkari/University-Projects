//package hu.kkari.client;
//
//import hu.kkari.chatUtil.messageManagement.Message;
//import hu.kkari.server.connectionManagement.MainReactor;
//
//import java.io.IOException;
//import java.util.Queue;
//import java.util.concurrent.ConcurrentLinkedQueue;
//
//import org.junit.After;
//import org.junit.Before;
//import org.junit.Test;
//
//public class ClientConnectionHandlerTest {
//	
//	Message regSuccessful = new Message("system", "registration successful"),
//			regNotSuccessful = new Message("system", "registration not successful"),
//			loginSuccessful = new Message("system", "login successful"), 
//			loginNotSuccessful = new Message("system", "login not successful"),
//			invalidTypeError = new Message("error", "invalid type");
//	int port = 9010;
//	MainReactor mr;
//	Queue<Message> recQ;
//	Queue<Message> sendQ;
//	ClientConnectionHandler cch;
//	
//	@Before
//	public void setUp() throws IOException {
//		 mr = new MainReactor(port);
//		 recQ = new ConcurrentLinkedQueue<Message>();
//		 sendQ = new ConcurrentLinkedQueue<Message>();
//		 Thread mrThread = new Thread(mr);
//		 Thread cchThread;
//		 mrThread.setName("mrThread");
//		 mrThread.start();
//		 try {
//			Thread.sleep(100);
//		} catch (InterruptedException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//		}
//		 cch = new ClientConnectionHandler(recQ, sendQ, "127.0.0.1", port); 
//		 cchThread = new Thread(cch);
//		 cchThread.setName("cch");
//		 cchThread.start();
//	}
//	
//	@Test
//	public void test() throws InterruptedException {
//		Message tmp = new Message("register", "bela");
//		sendQ.add(tmp);
//		Thread.sleep(1000);
//		System.out.println(recQ.poll().getMessage());
//		sendQ.add(new Message("login", "bela"));
//		Thread.sleep(1000);
//		System.out.println(recQ.poll().getMessage());
//	}
//	
//	 @After
//	 public void tearDown() throws IOException {
//		 mr.shutdown();
//	 }
//
//}
