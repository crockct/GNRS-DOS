package com.ketonax.networkLogic;

import android.util.Log;

import com.ketonax.drop_it.MainActivity;
import com.ketonax.message_package.DropMessage;

import edu.rutgers.winlab.jmfapi.GUID;

//System.arraycopy(src, pos, dst, pos, size);

/**
 * Created by wontoniii on 10/24/13.
 */
public class NetworkMessage {

	public final static byte DROP_MESSAGE_TYPE = 0;
	public final static byte REQ_MESSAGE_TYPE = 1;

	public static int byteArrayToInt(byte buffer[], int pos){
		int ret = 0;
		ret += buffer[pos] & 0x000000FF << 24;
		ret += (buffer[pos+1] & 0x000000FF) << 16;
		ret += (buffer[pos+2] & 0x000000FF) << 8;
		ret += (buffer[pos+3] & 0x000000FF);
		return ret;
	}

	public static byte[] intToByteArray(int a) {
		byte[] ret = new byte[4];
		ret[3] = (byte) (a & 0xFF);
		ret[2] = (byte) ((a >> 8) & 0xFF);
		ret[1] = (byte) ((a >> 16) & 0xFF);
		ret[0] = (byte) ((a >> 24) & 0xFF);
		return ret;
	}

	public static byte[] genDropMessage(DropMessage dm, int reqID){
		byte ret[] = new byte[1+4+4+4+4+4+ dm.getMessageHeader().length()+  dm.getMessageBody().length()];
		ret[0] = DROP_MESSAGE_TYPE;
		System.arraycopy(intToByteArray(dm.getMessageID()), 0, ret, 1, 4);
		System.arraycopy(intToByteArray(dm.getLocationGUID()), 0, ret, 5, 4);
        System.arraycopy(intToByteArray(reqID), 0, ret, 9, 4);
		System.arraycopy(intToByteArray(dm.getMessageHeader().length()), 0, ret, 13, 4);
		System.arraycopy(intToByteArray(dm.getMessageBody().length()), 0, ret, 17, 4);
		System.arraycopy(dm.getMessageHeader().getBytes(), 0, ret, 21, dm.getMessageHeader().length());
		System.arraycopy(dm.getMessageBody().getBytes(), 0, ret, 21+dm.getMessageHeader().length(), dm.getMessageBody().length());
		return ret;
	}

	public static DropMessage parseDropMessage(byte buffer[]){
		String header, msg;
		int id, locGUID, headerLength, bodyLentgh;
		id = byteArrayToInt(buffer, 1);
		locGUID = byteArrayToInt(buffer, 5);
		headerLength = byteArrayToInt(buffer, 13);
		bodyLentgh = byteArrayToInt(buffer, 17);
		header = new String(buffer, 21,headerLength);
		msg = new String(buffer, 21+headerLength, bodyLentgh);
		DropMessage ret = new DropMessage(header, msg, locGUID);
		ret.setMessageID(id);
		return ret;
	}

    public static int parseReqIDDropMessage(byte buffer[]){
        int reqID;
        reqID = byteArrayToInt(buffer, 1);
        return reqID;
    }

	public static byte[] genReqMessagge(GUID locGUID, GUID srcGUID, int reqID){
		byte ret[] = new byte[1+4+4+4];
		ret[0] = REQ_MESSAGE_TYPE;
		System.arraycopy(intToByteArray(locGUID.getGUID()), 0, ret, 1, 4);
		System.arraycopy(intToByteArray(srcGUID.getGUID()), 0, ret, 5, 4);
        System.arraycopy(intToByteArray(reqID), 0, ret, 9, 4);
		return ret;
	}

	public static void parseReqMessage(byte buffer[], GUID sg, GUID lg){
		Log.d(MainActivity.APPTAG, "Parser received new message");
		StringBuilder sb = new StringBuilder();
		for (int i = 0; i<9; i++) {
			sb.append(String.format("%02X ", buffer[i]));
		}
		Log.d(MainActivity.APPTAG, "Received " + sb.toString());
		int locGuid, srcGUID;
		locGuid = byteArrayToInt(buffer, 1);
		srcGUID = byteArrayToInt(buffer, 5);
		Log.d(MainActivity.APPTAG, "Parsed int " + locGuid);
		Log.d(MainActivity.APPTAG, "Parsed int " + srcGUID);
		sg.setGUID(srcGUID);
		lg.setGUID(locGuid);
	}

    public static int parseReqMessageReqID(byte buffer[]){
        Log.d(MainActivity.APPTAG, "Parser received new message");
        int reqID;
        reqID = byteArrayToInt(buffer, 9);
        return reqID;
    }
}
