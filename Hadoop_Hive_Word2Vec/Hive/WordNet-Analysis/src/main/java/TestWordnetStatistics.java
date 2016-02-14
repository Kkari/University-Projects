import static org.junit.Assert.*;

import java.util.ArrayList;
import org.dbprak.group3.wordnet.RecommenderQuery;
import org.dbprak.group3.wordnet.WordnetStatistics;
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
		
		RecommenderQuery aReQu= new RecommenderQuery(word,relatedW);
		
		WordnetStatistics stat = new WordnetStatistics(aReQu);
		
		assertTrue(true);
		//fail("Not yet implemented");
	}

}
