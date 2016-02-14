package hu.kkari.server;

import static org.junit.Assert.assertTrue;
import hu.kkari.chatUtil.messageManagement.ContinuousJsonParser;
import hu.kkari.chatUtil.messageManagement.Message;
import hu.kkari.server.connectionManagement.MainReactor;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.Socket;
import java.nio.CharBuffer;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class MainReactorTest {
	
	Message regSuccessful = new Message("server", "system", "registration successful"),
			regNotSuccessful = new Message("server", "system", "registration not successful"),
			loginSuccessful = new Message("server",  "system", "login successful"), 
			loginNotSuccessful = new Message("server", "system", "login not successful"),
			invalidTypeError = new Message("error", "invalid type");
	ContinuousJsonParser cjp;
	MainReactor mr;
	
	Socket alice;
	Socket bob;
	
	DataOutputStream aliceOut;
	OutputStream aliceOutToServer;
	
	OutputStream bobOutToServer;
	DataOutputStream bobOut;
	
	InputStream aliceInFromServer;
	InputStreamReader aliceSr;
	DataInputStream aliceIn;
	
	InputStream boBInFromServer;
	InputStreamReader bobSr;
	DataInputStream bobIn;
	
	CharBuffer charB;
	byte[] bb = new byte[1000];
	int port = 9090;
	
	
	 @Before
	 public void setup() throws IOException {
		
		 cjp = new ContinuousJsonParser();
		 mr = new MainReactor(port);
		 alice = new Socket("127.0.0.1", port);
		 bob = new Socket("127.0.0.1", port);
		 
		 new Thread(mr).start();
		 
		 aliceOutToServer = alice.getOutputStream();
         aliceOut = new DataOutputStream(aliceOutToServer);
         
         bobOutToServer = bob.getOutputStream();
         bobOut = new DataOutputStream(bobOutToServer);
         
         aliceInFromServer = alice.getInputStream();
         aliceIn =  new DataInputStream(aliceInFromServer);
         aliceSr = new InputStreamReader(aliceIn);
         
         boBInFromServer = bob.getInputStream();
         bobIn =  new DataInputStream(boBInFromServer);
         bobSr = new InputStreamReader(bobIn);
        
         charB = CharBuffer.allocate(400);
	 }
	 
	 @After
	 public void tearDown() throws IOException {
		 bobOut.close();
		 aliceOut.close();
		 mr.shutdown();
		 alice.close();
		 bob.close();
	 }
	 
	 @Test
	 public void invalidTypeTest() throws IOException, InterruptedException {
		 
         aliceOut.writeUTF("{\"sender\":\"Alice\"}");

         bb = new byte[1000];
		 aliceInFromServer.read(bb);
         assertTrue(cjp.evaluateBuffer(bb).Equals(invalidTypeError));
   	 }
	 
	 @Test
	 public void testSend() throws IOException, InterruptedException {
		 
		 Message m = new Message("register", "pass");
		 m.setSender("Alice");
		 aliceOut.write(m.Serialize());
		 aliceInFromServer.read(bb);
		 
		 m.setType("login");
		 aliceOut.write(m.Serialize());
		 aliceInFromServer.read(bb);
		 
		 m.setType("register");
		 m.setSender("Bob");
		 bobOut.write(m.Serialize());
		 boBInFromServer.read(bb);
		 
		 m.setType("login");
		 bobOut.write(m.Serialize());
		 boBInFromServer.read(bb);
		 
		 m.setType("msg");
		 m.setSender("Alice");
		 m.setRecipient("Bob");
		 m.setMessage("msg1");
		 aliceOut.write(m.Serialize());
		 
		 bb = new byte[1000];
		 boBInFromServer.read(bb);
		 assertTrue(cjp.evaluateBuffer(bb).Equals(m));
		 
		 m.setType("msg");
		 m.setSender("Bob");
		 m.setRecipient("Alice");
		 m.setMessage("msg2");
		 bobOut.write(m.Serialize());
		 
		 bb = new byte[1000];
		 aliceInFromServer.read(bb);
		 assertTrue(cjp.evaluateBuffer(bb).Equals(m));
		 

		 m.setType("msg");
		 m.setSender("Alice");
		 m.setRecipient("Bob");
		 m.setMessage("msg3");
		 aliceOut.write(m.Serialize());
		 
		 bb = new byte[1000];
		 boBInFromServer.read(bb);
		 assertTrue(cjp.evaluateBuffer(bb).Equals(m));
		 System.out.print("done");
		 
		 m.setType("msg");
		 m.setSender("Bob");
		 m.setRecipient("Alice");
		 m.setMessage("msg4");
		 bobOut.write(m.Serialize());
		 
		 bb = new byte[1000];
		 aliceInFromServer.read(bb);
		 assertTrue(cjp.evaluateBuffer(bb).Equals(m));
		 
	 }
}
