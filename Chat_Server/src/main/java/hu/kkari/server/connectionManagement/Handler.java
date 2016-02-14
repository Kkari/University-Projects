package hu.kkari.server.connectionManagement;

import hu.kkari.chatUtil.messageManagement.ContinuousMessageParser;
import hu.kkari.chatUtil.messageManagement.Message;

import java.io.IOException;


public interface Handler {
	public void sendString(Message msg) throws IOException;
	public ContinuousMessageParser getMessageProcessor();
}
