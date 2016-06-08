package com.ketonax.message_package;

public class MessageFormatException extends Exception{
	
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	public MessageFormatException() {
		
		super("Improper message format!");
		
	}
	
	public MessageFormatException(String message){
		
		super(message);
		
	}

}
