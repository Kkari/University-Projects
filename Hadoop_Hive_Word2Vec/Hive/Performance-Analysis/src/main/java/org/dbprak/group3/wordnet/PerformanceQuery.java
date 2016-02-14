package org.dbprak.group3.wordnet;

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;

public class PerformanceQuery {
	
	public static final int LIMIT = 100;
	private ArrayList<String> related_words = new ArrayList<String>();
	private String word;
	private String table; // table used in query
	long estimatedTime;
	

	public PerformanceQuery(String query) {
		Connection con = HiveConnection.getConnection();
		ResultSet rs = null;
		try {
			Statement stmt = con.createStatement();
		
			long startTime = System.currentTimeMillis();    
			rs = stmt.executeQuery(query);  
			estimatedTime = System.currentTimeMillis() - startTime;
		     
		     while(rs.next()){
		    	String a = rs.getString(1);
				related_words.add(a);
			}
		} catch (SQLException e) {
			e.printStackTrace();
		}
	}
	public PerformanceQuery(String word, ArrayList<String> relatedW) {
		this.word = word;
		this.related_words = relatedW;

	}
	
	public long getEstimatedTime() {
		return estimatedTime;
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
