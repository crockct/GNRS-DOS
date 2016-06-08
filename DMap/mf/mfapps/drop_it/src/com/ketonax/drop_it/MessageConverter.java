package com.ketonax.drop_it;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.ArrayList;

import com.ketonax.message_package.DropMessage;

public class MessageConverter {

	public static byte[] toByteArray(ArrayList<DropMessage> messageList) {

		byte[] buff = null;

		ByteArrayOutputStream baos = new ByteArrayOutputStream();

		try {

			ObjectOutputStream outputStream = new ObjectOutputStream(baos);
			outputStream.writeObject(messageList);

			buff = baos.toByteArray();

			outputStream.close();

		} catch (Exception e) {
			// TODO: handle exception
		}

		return buff;

	}

	@SuppressWarnings("unchecked")
	public static ArrayList<DropMessage> toArrayList(byte[] packet) {

		ArrayList<DropMessage> receivedList = null;

		try {

			ByteArrayInputStream bais = new ByteArrayInputStream(packet);

			ObjectInputStream ois = new ObjectInputStream(bais);
			receivedList = (ArrayList<DropMessage>) ois.readObject();

		} catch (Exception e) {
			// TODO: handle exception
		}

		return receivedList;

	}

}
