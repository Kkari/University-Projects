package org.dbprak.group3.wordnet;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;

public class HiveConnection {
	
	private static Connection conn;
	
	public static Connection getConnection() {
		if(conn == null) {
			try {
				Class.forName("org.apache.hive.jdbc.HiveDriver");			
				conn = DriverManager.getConnection("jdbc:hive2://ec2-54-229-212-152.eu-west-1.compute.amazonaws.com:10000/default", "", "");
			} catch (ClassNotFoundException e) {
				e.printStackTrace();
			} catch (SQLException e) {
				System.out.println("Connection Error: " + e.getMessage());
				e.printStackTrace();
			}
		}
		return conn;
	}
	
	public static void closeConnection() {
		if(conn != null)
			try {
				conn.close();
			} catch (SQLException e) {
				System.out.println("Connection Error: " + e.getMessage());
				e.printStackTrace();
			}
	}
}
