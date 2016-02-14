package hu.kkari.chatUtil.messageManagement;

import com.fasterxml.jackson.core.JsonProcessingException;


/**
 * @author kkari
 * Realizations of this interface transform the Messages to the chosen form of transmission.
 * This interface is tightly coupled with the ContinuousMessageParser interface.
 */
public interface Serializer {
	public byte[] serialize(Message msg) throws JsonProcessingException;
}
