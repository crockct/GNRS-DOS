package com.ketonax.message_package;

import java.io.Serializable;
import java.util.Calendar;
import java.util.Random;

import edu.rutgers.winlab.jmfapi.*;

public class DropMessage implements Serializable {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1048070917889360190L;
	int messageID;
	int locationGUID;
	int srcGUID;
	String messageHeader, messageBody;
	String messageTimeStamp;

	boolean hasMessage;

	public DropMessage(String messageHeader, String messageBody) {

		this.messageHeader = messageHeader;
		this.messageBody = messageBody;
		hasMessage = true;

		setTimeStamp();
		createMessageID();

	}

	public DropMessage(String messageHeader, String messageBody,
			int locationGUID) {

		this(messageHeader, messageBody);
		this.locationGUID = locationGUID;

	}

	public void setLocationGUID(int locationGUID) {
		this.locationGUID = locationGUID;
	}

	public void setTimestamp(String messageTimestamp) {
		this.messageTimeStamp = messageTimestamp;
	}

	public void setMessageID(int messageID) {
		this.messageID = messageID;
	}

	public int getMessageID() {
		return messageID;
	}

	public String getMessageHeader() {
		return messageHeader;
	}

	public String getMessageBody() {

		return messageBody;

	}

	public int getLocationGUID() {
		return locationGUID;
	}

	public String getTimeStamp() {
		return messageTimeStamp;
	}

	public int getSrcGUID() {
		return srcGUID;
	}

	public void setSrcGUID(int srcGUID) {
		this.srcGUID = srcGUID;
	}

	public boolean containsMessage() {

		return hasMessage;

	}

	@Override
	public String toString() {

		return messageHeader;

	}

	// Private Methods
	private void setTimeStamp() {

		Calendar c = Calendar.getInstance();

		messageTimeStamp = c.getTime().toString();

	}

	private void createMessageID() {

		Random randomID = new Random();

		messageID = randomID.nextInt();

	}

	@Override
	public boolean equals(Object o) {
		DropMessage other = (DropMessage)o;
		return messageID == other.getMessageID();
	}
}
