package hu.kkari.chatUtil.messageManagement;

import java.io.IOException;
import java.io.StringReader;
import java.nio.charset.Charset;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;

/**
 * @author kkari
 * This class is responsible to gather JSON object from the incoming stream from a socketChannel, each handler has such an object
 */
public class ContinuousJsonParser implements ContinuousMessageParser, Serializer{
	
	private StringBuilder innerBuffer = new StringBuilder();
	
	
	/**
	 * @return the next message in the buffer.
	 */
	@Override
	public synchronized Message evaluateBuffer(byte[] bytes) {
		innerBuffer.append(new String(bytes, Charset.forName("ISO-8859-1")));
		int[] index;
		if((index = findJson()) != null) {
			String tmp = innerBuffer.substring(index[0],index[1]);
			innerBuffer.replace(0,index[1], "");
			ObjectMapper obm = new ObjectMapper();
			try {
				return obm.readValue(new StringReader(tmp), Message.class);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		return null;
	}
	
	/**
	 * 
	 * @return the coordinates of the next pair of outer curly braces
	 */
	private synchronized int[] findJson() {
		
		String tmp = innerBuffer.toString();
		tmp.trim();
		int braceMonitor = 0;
		int[] retVals = new int[2];
		boolean gotBrace = false;
		
		for(int i = 0; i < tmp.length(); i++) {
			if(tmp.charAt(i) == '{') {
				if(braceMonitor == 0) retVals[0] = i;
				gotBrace = true;
				braceMonitor++;
			} else if((tmp.charAt(i) == '}') && gotBrace) {
				braceMonitor--;
			}
			
			if(braceMonitor == 0) {
				if (gotBrace) {
					retVals[1] = ++i;
					return retVals;
				}
			}
		}
		return null;
	}
	
	/**
	 * @return a byte sequence ready to be transferred over the network
	 */
	@Override
	public byte[] serialize(Message msg) throws JsonProcessingException {
		ObjectMapper obm = new ObjectMapper();
		return obm.writeValueAsBytes(msg);
	}
}
