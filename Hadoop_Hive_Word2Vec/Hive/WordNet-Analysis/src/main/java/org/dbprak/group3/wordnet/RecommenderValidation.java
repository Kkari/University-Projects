package org.dbprak.group3.wordnet;

import java.util.ArrayList;

import org.apache.log4j.Level;
import org.apache.log4j.Logger;

import net.sf.extjwnl.JWNLException;
import net.sf.extjwnl.data.IndexWord;
import net.sf.extjwnl.data.POS;
import net.sf.extjwnl.dictionary.Dictionary;

public class RecommenderValidation {
	
	public static void main(String args[]) {
		// disable verbose logging
        Logger rootLogger = Logger.getRootLogger();
        rootLogger.setLevel(Level.ERROR);
		
		if(args.length != 2) {
			System.out.println("Usage: ");
			System.out.println("java -jar bla.jar <table_name> <num_queries>");
			System.exit(0);
		}
		
		String table = args[0];
		int num_words = Integer.parseInt(args[1]);
		
		System.out.println("Start running queries for " + num_words + " words and analyse 100 most related results for each.");
		System.out.println();
		
		// get random adjectives from wordnet
		ArrayList<String> queryWords = new ArrayList<String>();
		Dictionary dictionary = null;
		
		try {
			dictionary = Dictionary.getDefaultResourceInstance();
		    for(int i = 0; i < num_words ;i++){
		    	IndexWord random_adjective = dictionary.getRandomIndexWord(POS.ADJECTIVE);
		    	queryWords.add(random_adjective.getLemma());
		    }
		    
		} catch (JWNLException e) {
			System.out.println("JWNL Error: " + e.getMessage());
			e.printStackTrace();
		}
		
		// run queries
		ArrayList<RecommenderQuery> queries = new ArrayList<RecommenderQuery>();
		for(int i = 0; i < num_words; i++) {
			System.out.print((i+1) + ". Run Query for '" + queryWords.get(i) + "' ...");
			RecommenderQuery query = new RecommenderQuery(queryWords.get(i), table);
			if (query.getResultSize() != 0) {
				queries.add(query);
				System.out.println();
			} else {
				System.out.println(" no results!");
			}
		}
		
		// create statistics
		ArrayList<WordnetStatistics> stats = new ArrayList<WordnetStatistics>(queries.size());
		for(int i = 0; i < queries.size(); i++) {
			stats.add(new WordnetStatistics(queries.get(i)));
			System.out.println("----------------------------");
			System.out.println(stats.get(i));
		}
		
		System.out.println("----------------------------");
		System.out.println("----------------------------");
		System.out.println(WordnetStatistics.toString(stats));
		
		HiveConnection.closeConnection();
	}
}