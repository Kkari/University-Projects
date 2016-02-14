package hu.kkari.server.clientManagement;

import static org.junit.Assert.assertTrue;
import hu.kkari.chatUtil.messageManagement.Message;
import hu.kkari.server.connectionManagement.DummyHandler;

import java.io.IOException;

import javax.naming.NameNotFoundException;

import org.junit.Test;

public class ClientRecorderTest {

	@Test(expected=NameNotFoundException.class)
	public void test() throws NameNotFoundException, IOException {
		ClientRecorder clr = ClientRecorder.getInstance();
		assertTrue(clr.register("sanyi", "lol"));
		assertTrue(clr.login("sanyi", "lol", new DummyHandler()));
		clr.getHandler("sanyi").sendString(new Message());
		clr.logout("sanyi");
		clr.getHandler("sanyi");
	}

}
