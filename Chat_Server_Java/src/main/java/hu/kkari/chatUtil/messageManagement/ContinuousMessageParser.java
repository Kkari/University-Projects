package hu.kkari.chatUtil.messageManagement;

import com.fasterxml.jackson.core.JsonProcessingException;


/**
 * @author kkari
 * As this new kind of IO is stream based instead of block, it is important to fetch the
 * individual objects from the stream of whatever comes through. Realizations of this interface
 * are responsible for this task.
 */
public interface ContinuousMessageParser {
	
	public Message evaluateBuffer(byte[] bytes);

	byte[] serialize(Message msg) throws JsonProcessingException;

}
