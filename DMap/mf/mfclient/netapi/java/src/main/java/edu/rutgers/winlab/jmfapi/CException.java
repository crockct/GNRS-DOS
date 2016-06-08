package edu.rutgers.winlab.jmfapi;

/*
 *
 * File: CException.java
 * Author: Francesco Bronzino
 *
 * Description: Exception class that is used to handle exceptions occurred in jni code
 *
 */

/**
 * Exception class that is used to handle exceptions occurred in jni or c code
 */
public class CException extends Exception{

	/**
	 *
	 */
	public CException(){
		super();
	}

	/**
	 * See normal Java Exceptions as reference
	 * @param message contains the error message
	 */
	public CException(String message){
		super(message);
	}

	/**
	 * See normal Java Exceptions as reference
	 * @param message contains the error message
	 * @param cause
	 */
	public CException(String message, Throwable cause){
		super(message,cause);
	}
}
