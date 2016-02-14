package hu.kkari.server.connectionManagement;

import hu.kkari.chatUtil.messageManagement.ContinuousMessageParser;
import hu.kkari.chatUtil.messageManagement.Message;

import java.io.IOException;


/**
 * @author kkari
 * Dummy handler for testing purposes
 */
public class DummyHandler implements Handler {

	@Override
	public void sendString(Message msg) throws IOException {
		System.out.println("String Sent");
	}
	
	@Override
	public ContinuousMessageParser getMessageProcessor() {
		// TODO Auto-generated method stub
		return null;
	}

}
