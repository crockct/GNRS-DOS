package com.ketonax.message_package;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.ArrayList;

public class MFByteConverter {

	static String ID, location;
	static String header, body;
	static String timeStamp;

	public static byte[] messageToByteArray(ArrayList<DropMessage> messageList) {

		byte[] byteArray = null;

		try {

			ByteArrayOutputStream baos = new ByteArrayOutputStream();
			ObjectOutputStream oos = new ObjectOutputStream(baos);

			oos.writeObject(messageList);

			byteArray = baos.toByteArray();

			baos.close();
			oos.close();

		} catch (Exception e) {
			e.printStackTrace();
		}

		return byteArray;
	}

	@SuppressWarnings("unchecked")
	public static ArrayList<DropMessage> byteArrayToMessage(byte[] messageBytes) {

		ArrayList<DropMessage> retrievedList = null;

		try {

			ObjectInputStream ois = new ObjectInputStream(
					new ByteArrayInputStream(messageBytes));

			retrievedList = (ArrayList<DropMessage>) ois.readObject();

			ois.close();

		} catch (Exception e) {
			e.printStackTrace();
		}

		return retrievedList;

	}

}
