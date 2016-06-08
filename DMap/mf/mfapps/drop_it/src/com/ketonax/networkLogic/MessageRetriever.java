package com.ketonax.networkLogic;

import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.ketonax.drop_it.MainActivity;

import edu.rutgers.winlab.jmfapi.*;

/**
 * Created by wontoniii on 10/21/13.
 * Given an MF socket and a "" to report messages listen for incoming messages
 */
public class MessageRetriever  implements Runnable{

	JMFAPI mfsocket;
	boolean cont;
	boolean running;
	Handler serviceHandler;
	static int MAX_BUFFER = 1024*1204*3;

	MessageRetriever(JMFAPI sock, Handler serviceHandler){
		mfsocket = sock;
		cont = false;
		running = false;
		this.serviceHandler = serviceHandler;
	}

	public void run(){
		Log.d(MainActivity.APPTAG, "Message retriever started");
		byte buffer[] = new byte[MAX_BUFFER];
		cont = true;
		running = true;
        GUID senderGUID = new GUID();
		int size;
		try{
			while(cont){
				Log.d(MainActivity.APPTAG, "Message retriever ready to receive new message");
				size = mfsocket.jmfrecv_blk(senderGUID, buffer, MAX_BUFFER);
				Log.d(MainActivity.APPTAG, "Message retriever received new message from: " + senderGUID.getGUID());
				StringBuilder sb = new StringBuilder();
				for (int i = 0; i<size; i++) {
					sb.append(String.format("%02X ", buffer[i]));
				}
				Log.d(MainActivity.APPTAG, "Received " + sb.toString());
				if(size>0){
					if(buffer[0] == NetworkMessage.REQ_MESSAGE_TYPE){
						Log.d(MainActivity.APPTAG, "Message retriever received remote request");
						GUID sg = new GUID();
						GUID lg = new GUID();
						NetworkMessage.parseReqMessage(buffer, sg, lg);
                        Message msg = Message.obtain(null, MFInternetService.REMOTE_REQUEST, sg.getGUID(), lg.getGUID());
                        msg.obj = new Integer(NetworkMessage.parseReqMessageReqID(buffer));
						serviceHandler.sendMessage(msg);
					}
					else if(buffer[0] == NetworkMessage.DROP_MESSAGE_TYPE){
						Log.d(MainActivity.APPTAG, "Message retriever received remote dropped message");
                        Message msg = Message.obtain(null, MFInternetService.REMOTE_DROP, NetworkMessage.parseDropMessage(buffer));
                        msg.arg1 = NetworkMessage.parseReqIDDropMessage(buffer);
						serviceHandler.sendMessage(msg);
					}
					else{
						Log.e(MainActivity.APPTAG, "Message retriever received unknown message of size " + size);
					}
				}
				else{
					Log.e(MainActivity.APPTAG, "Message retriever problems");
				}
			}
		} catch (JMFException e){
			Log.d(MainActivity.APPTAG, e.toString());
		} catch (Exception e){
			Log.d(MainActivity.APPTAG, e.toString());
		}
		finally {
			running = false;
		}
	}

	public void stopRunning(){
		cont = false;
	}
}

