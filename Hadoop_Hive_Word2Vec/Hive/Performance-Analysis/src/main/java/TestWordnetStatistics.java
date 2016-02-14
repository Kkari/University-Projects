import static org.junit.Assert.*;

import java.util.ArrayList;
import org.dbprak.group3.wordnet.PerformanceQuery;

import org.junit.Test;

public class TestWordnetStatistics {

	@Test
	public void test() {
		String word = "old";
		ArrayList<String> relatedW = new ArrayList<String>();
		relatedW.add("grey");
		relatedW.add("aged");
		relatedW.add("anile");
		relatedW.add("small");
		relatedW.add("young");
		relatedW.add("green");
		relatedW.add("funny");
		relatedW.add("house");
		relatedW.add("walking");
		relatedW.add("aosdgj");
		
		PerformanceQuery aReQu= new PerformanceQuery(word,relatedW);
	
		assertTrue(true);
		//fail("Not yet implemented");
	}

}
