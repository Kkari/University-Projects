package org.dbprak.group3.wordnet;

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;

public class RecommenderQuery {
	
	public static final int LIMIT = 100;
	private ArrayList<String> related_words = new ArrayList<String>();
	private String word;
	private String table; // table used in query
	
	
	public RecommenderQuery(String word, String table) {
		this.word = word;
		this.table = table;
		Connection con = HiveConnection.getConnection();
		ResultSet rs = null;
		try {
			Statement stmt = con.createStatement();
			
			String query = "SELECT words2.word, cosineSimilarity(" + table + ".vector, words2.vector) " +
					"as dist FROM " + table + " " + 
					"JOIN " + table + " as words2 ON " + table + ".word = '" + word + "' AND " + table + ".id = words2.id " + 
					"ORDER BY dist DESC LIMIT " + LIMIT;
			
		     rs = stmt.executeQuery(query);
		     while(rs.next()){
		    	String a = rs.getString(1);
				related_words.add(a);
			}
		} catch (SQLException e) {
			e.printStackTrace();
		}
	}
	public RecommenderQuery(String word, ArrayList<String> relatedW) {
		this.word = word;
		this.related_words = relatedW;

	}
	
	public String getWord() {
		return word; 
	}
	
	public String getTable() {
		return table;
	}
	
	public String getRelated(int i) {
		return related_words.get(i);
	}
	
	public int getResultSize() {
		return related_words.size();
	}

}
