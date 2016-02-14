package hu.kkari.chatUtil.messageManagement;

import static org.junit.Assert.assertTrue;
import hu.kkari.chatUtil.messageManagement.ContinuousJsonParser;
import hu.kkari.chatUtil.messageManagement.Message;

import org.junit.Test;

public class ContinuousJsonParserTest {
	@Test
	public void outerTest() {
		
		ContinuousJsonParser cmp = new ContinuousJsonParser();
		String s = "{\"sender\":\"Krieger\"}";
	
		Message m = cmp.evaluateBuffer(s.substring(0, s.length()/2).getBytes());
		assertTrue(m == null);
		
		Message m2 = cmp.evaluateBuffer(s.substring(s.length()/2,s.length()).getBytes());
		
		assertTrue(m2.getSender().equals("Krieger"));
	}

}
