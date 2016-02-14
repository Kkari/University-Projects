package hu.kkari.server.clientManagement;

import hu.kkari.server.connectionManagement.Handler;

import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

import javax.naming.NameNotFoundException;

public class ClientRecorder {
	
	private static ClientRecorder instance;
	private Map<String, String> registeredClients = new ConcurrentHashMap<>();
	private Map<String, Handler> loggedInClients = new ConcurrentHashMap<>();
	
	private ClientRecorder(){}

	public static synchronized ClientRecorder getInstance()
	{
		if (instance == null)
			instance = new ClientRecorder();
		return instance;
	}
	
	public boolean register(String name, String pass)
	{
		if(!registeredClients.containsKey(name)) {
			registeredClients.put(name, pass);
			return true;
		}
		
		return false;
	}
	
	public boolean login(String name, String pass, Handler h) {
		if(registeredClients.containsKey(name) && !loggedInClients.containsKey(name)) {
			if(registeredClients.get(name).equals(pass));
				loggedInClients.put(name, h);
			return true;
		}
		return false;
	}
	
	public void logout(String name) {
		loggedInClients.remove(name);
	}
	
	public Handler getHandler(String name) throws NameNotFoundException {
		synchronized (loggedInClients) {
			if(loggedInClients.containsKey(name)) {
				return loggedInClients.get(name);
			} else throw new NameNotFoundException();
		}
	}
	
	public Set<String> getAllLoggedInClients() {
		return loggedInClients.keySet();
	}
}
