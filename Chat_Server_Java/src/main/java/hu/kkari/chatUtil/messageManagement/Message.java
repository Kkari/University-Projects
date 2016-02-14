package hu.kkari.chatUtil.messageManagement;

import com.fasterxml.jackson.core.JsonProcessingException;


/**
 * @author kkari
 * This is the message, whom instances will be handled across the server and the client.
 * One can plug in a Serializer, that is Matching the Parser interface, in order to make
 * communication with different approaches possible like POJO or JSON.
 */
public class Message {
	
	private String sender = "unknown", recipient = "all", message = "unknown", type = "unknown";
	private Serializer sr = new ContinuousJsonParser();
	
	public Message() {}
	public Message(String type_, String message_) {
		this();
		type = type_;
		message = message_;
	}
	
	public Message(String sender_, String type_, String message_) {
		this(type_,message_);
		sender = sender_;
	}
	
	public String getSender() {
		return sender;
	}
	
	public void setSender(String sender_) {
		this.sender = sender_;
	}
	
	public String getRecipient() {
		return recipient;
	}
	
	public void setRecipient(String recipient_) {
		recipient = recipient_;
	}
	
	public String getMessage() {
		return message;
	}
	
	public void setMessage(String str) {
		message = str;
	}
	
	public String getType() {
		return type;
	}
	
	public void setType(String type_) {
		type = type_;
	}
	
	public byte[] Serialize() {
		try {
			return sr.serialize(this);
		} catch (JsonProcessingException e) {
			return "{\"message\":\"serialize error\"}".getBytes();
		}
	}
	
	public boolean Equals(Message that) {
		return (this.message.equals(that.message) &&
				this.sender.equals(that.sender) &&
				this.recipient.equals(that.recipient) &&
				this.type.equals(that.type));
	}
}
