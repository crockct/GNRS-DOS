package edu.rutgers.winlab.jmfapi;

/*
 *
 * File: JMFException.java
 * Author: Francesco Bronzino
 *
 * Description: Exception class that is used to handle exceptions occurred in MobilityFirst's Java API
 *
 */

/**
 * Exception class that is used to handle exceptions occurred in MobilityFirst's Java API
 */
public class JMFException extends Exception {

	int c;

	/**
	 *
	 */
    public JMFException(){
        super();
    }

	/**
	 * See normal Java Exceptions as reference
	 * @param message contains the error message
	 */
    public JMFException(String message) {
        super(message);
    }

	/**
	 * See normal Java Exceptions as reference
	 * @param message contains the error message
	 */
	public JMFException(String message, int code) {
		super(message);
		c = code;
	}

	/**
	 * See normal Java Exceptions as reference
	 * @param message contains the error message
	 * @param cause
	 */
    public JMFException(String message, Throwable cause) {
        super(message, cause);
    }

	/**
	 * Get the error code
	 * @return the error code
	 */
	public int getCode() {
		return c;
	}

	/**
	 * Set the error code
	 * @param c the error code to be set
	 */
	public void setCode(int c) {
		this.c = c;
	}
}
