package org.dbprak.group3.wordnet;

import java.text.DecimalFormat;
import java.util.Collection;
import java.util.List;

import net.sf.extjwnl.JWNLException;
import net.sf.extjwnl.data.IndexWord;
import net.sf.extjwnl.data.IndexWordSet;
import net.sf.extjwnl.data.POS;
import net.sf.extjwnl.data.PointerUtils;
import net.sf.extjwnl.data.Synset;
import net.sf.extjwnl.data.Word;
import net.sf.extjwnl.data.list.PointerTargetNode;
import net.sf.extjwnl.data.list.PointerTargetNodeList;
import net.sf.extjwnl.dictionary.Dictionary;

public class WordnetStatistics { 
	
	private RecommenderQuery query;
	private Dictionary dictionary;
	private static DecimalFormat fmt = new DecimalFormat("#.##");
	
	// POS Counter
	private int notFoundCounter = 0;
	private int adjectiveFoundCounter= 0;
	private int verbFoundCounter= 0;
	private int adverbFoundCounter= 0;
	private int nounFoundCounter= 0;
	
	// Synonym Antonym Counter
	private int synonymFoundCounter = 0;
	private int antonymFoundCounter = 0;
	private int noRelationCounter = 0;
	
	
	public WordnetStatistics(RecommenderQuery query) {
		this.query = query;
		try {
			
			this.dictionary = Dictionary.getDefaultResourceInstance();
			String queryWord = query.getWord();
			
		    for(int i = 0; i < RecommenderQuery.LIMIT; i++) {
		    	String relatedWord =  query.getRelated(i);
		    	
		    	IndexWordSet iWords = dictionary.lookupAllIndexWords(relatedWord);
		    	if(iWords.isValidPOS(POS.ADJECTIVE)){
		    		adjectiveFoundCounter ++;
		    	}
		    	if(iWords.isValidPOS(POS.NOUN)){
		    		nounFoundCounter ++;
		    	}
		    	if(iWords.isValidPOS(POS.VERB)){
		    		verbFoundCounter ++;
		    	}
		    	if(iWords.isValidPOS(POS.ADVERB)){
		    		adverbFoundCounter ++;
		    	}
		    	if(iWords.size() == 0) {
		    		notFoundCounter++;
		    	}

		    	
		    	IndexWord adjective = iWords.getIndexWord(POS.ADJECTIVE);
		    	
		    	if(iWords.isValidPOS(POS.ADJECTIVE)){
		        	Boolean isSynonym = false;
		        	Boolean isAntonym = false;
		    		
			    	List<Synset> senses = adjective.getSenses();
			        for (Synset synset : senses) {
			        	PointerTargetNodeList synonyms = PointerUtils.getSynonyms(synset);
			        	PointerTargetNodeList antonyms = PointerUtils.getAntonyms(synset);
			
			        	for(PointerTargetNode node : synonyms) {
			        		for(Word synonym : node.getSynset().getWords()) {
			        			String syn = synonym.getLemma();
			        			if(syn.equalsIgnoreCase(queryWord)) {
			        				//System.out.println(queryWord + " ist Synonym zu " + relatedWord);
			        				isSynonym = true;
			        			}
			        				
			        		}
			        	}
			        	
			        	for(PointerTargetNode node : antonyms) {
			        		for(Word antoym : node.getSynset().getWords()) {
			        			String ant = antoym.getLemma();
			        			if(ant.equalsIgnoreCase(queryWord)) {
			        				//System.out.println(queryWord + " ist Antonym zu " + relatedWord);
			        				isAntonym = true;
			        			}
			        		}
			        	}
			        }

			    	
			    	if(isSynonym) 
			    		synonymFoundCounter++;
			    	if(isAntonym) 
			    		antonymFoundCounter++;
			    	if(!isSynonym && !isAntonym)
			    		noRelationCounter++;
		    	}
		    	
		    }
		} catch (JWNLException e) {
			System.out.println("JWNL Error: " + e.getMessage());
			e.printStackTrace();
		}
	}
	
	public String toString() {
		String result = "";
		
		result += "Query Word: " + query.getWord() + "\n\n";
		result += "    Nouns: " + fmt.format(nounFoundCounter/(double)RecommenderQuery.LIMIT*100) + "% (" + nounFoundCounter + ")\n";
		result += "    Verbs: " + fmt.format(verbFoundCounter/(double)RecommenderQuery.LIMIT*100) + "% (" + verbFoundCounter + ")\n";
		result += "    Adjectives: " + fmt.format(adjectiveFoundCounter/(double)RecommenderQuery.LIMIT*100) + "% (" + adjectiveFoundCounter + ")\n";
		result += "    Adverbs: " + fmt.format(adverbFoundCounter/(double)RecommenderQuery.LIMIT*100) + "% (" + adverbFoundCounter + ")\n";
		result += "    Not found: " + fmt.format(notFoundCounter/(double)RecommenderQuery.LIMIT*100) + "% (" + notFoundCounter + ")\n\n";
		
		result += "    Adjective Relations (out of " + adjectiveFoundCounter + ") :\n";
		result += "        Synonyms: " + fmt.format(synonymFoundCounter/(double)adjectiveFoundCounter*100) + "% (" + synonymFoundCounter + ")\n";
		result += "        Antonyms: " + fmt.format(antonymFoundCounter/(double)adjectiveFoundCounter*100) + "% (" + antonymFoundCounter + ")\n";
		result += "        None: " + fmt.format(noRelationCounter/(double)adjectiveFoundCounter*100)+ "% (" + noRelationCounter + ")\n";

		return result;
	}
	
	public static String toString(Collection<WordnetStatistics> statistics) {
		int nounFoundCounter = 0;
		int verbFoundCounter = 0;
		int adjectiveFoundCounter = 0;
		int adverbFoundCounter = 0;
		int notFoundCounter = 0;
		
		int synonymFoundCounter = 0;
		int antonymFoundCounter = 0;
		int noRelationCounter = 0;
		
		for(WordnetStatistics stat : statistics) {
			nounFoundCounter += stat.nounFoundCounter;
			verbFoundCounter += stat.verbFoundCounter;
			adjectiveFoundCounter += stat.adjectiveFoundCounter;
			adverbFoundCounter += stat.adverbFoundCounter;
			notFoundCounter += stat.notFoundCounter;
			
			synonymFoundCounter += stat.synonymFoundCounter;
			antonymFoundCounter += stat.antonymFoundCounter;
			noRelationCounter += stat.noRelationCounter;
		}
		
		int numTotal = RecommenderQuery.LIMIT * statistics.size();
		
		String result = "";
		
		result += "Summary Statistics: \n\n";
		result += "    Nouns: " + fmt.format(nounFoundCounter/(double)numTotal*100) + "% (" + nounFoundCounter + ")\n";
		result += "    Verbs: " + fmt.format(verbFoundCounter/(double)numTotal*100) + "% (" + verbFoundCounter + ")\n";
		result += "    Adjectives: " + fmt.format(adjectiveFoundCounter/(double)numTotal*100) + "% (" + adjectiveFoundCounter + ")\n";
		result += "    Adverbs: " + fmt.format(adverbFoundCounter/(double)numTotal*100) + "% (" + adverbFoundCounter + ")\n";
		result += "    Not found: " + fmt.format(notFoundCounter/(double)numTotal*100) + "% (" + notFoundCounter + ")\n\n";
		
		result += "    Adjective Relations (out of " + adjectiveFoundCounter + ") :\n";
		result += "        Synonyms: " + fmt.format(synonymFoundCounter/(double)adjectiveFoundCounter*100) + "% (" + synonymFoundCounter + ")\n";
		result += "        Antonyms: " + fmt.format(antonymFoundCounter/(double)adjectiveFoundCounter*100) + "% (" + antonymFoundCounter + ")\n";
		result += "        None: " + fmt.format(noRelationCounter/(double)adjectiveFoundCounter*100)+ "% (" + noRelationCounter + ")\n";
		
		return result;
	}
}
