package hu.kkari.server;

import hu.kkari.server.connectionManagement.MainReactor;

import java.io.IOException;

public class Main {
	public static void main(String[] args) {
		try {
	    	new Thread(new MainReactor(9090)).start();
	    } catch (IOException ex) {
	        ex.printStackTrace();
	    }
	}
}
