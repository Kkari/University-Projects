package hu.kkari.client;

import hu.kkari.chatUtil.messageManagement.Message;

import java.util.Queue;

import javax.swing.JTextArea;



/**
 * @author kkari
 *
 * This class updates the chatBox received in it's constructor, whenever there is a new message
 * in the receiving queue.
 */
public class GuiNetworkingInterface implements Runnable {

	private Queue<Message> recQ; 
	private JTextArea chatBox;
	
   public GuiNetworkingInterface(Queue<Message> recQ_, JTextArea chatBox_) {
	   this.recQ = recQ_;
	   this.chatBox = chatBox_;
	}
   
   
	@Override
	public void run() {
		   while (true) {
			   synchronized (recQ) {
				   
				   Message tmp = recQ.poll();
				   
				   while (tmp != null) {
					   chatBox.append("<" + tmp.getSender()  + ">:  " + tmp.getMessage() + "\n");
					   tmp = recQ.poll();
				   }
				   
				   try {
					recQ.wait();
				   } catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				   }
			   }
		   }
	}

}
