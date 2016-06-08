package com.ketonax.networkLogic;

import java.io.File;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.http.HttpResponse;
import org.apache.http.NameValuePair;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.message.BasicNameValuePair;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.support.v4.app.NotificationCompat;
import android.util.Log;

import com.ketonax.drop_it.MainActivity;
import com.ketonax.drop_it.R;
import com.ketonax.message_package.DropMessage;

import edu.rutgers.winlab.jmfapi.GUID;
import edu.rutgers.winlab.jmfapi.JMFAPI;
import edu.rutgers.winlab.jmfapi.JMFException;

import java.io.File;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Random;

import edu.rutgers.winlab.jmfapi.*;
import edu.rutgers.winlab.jgnrs.JGNRS;

/**
 * Created by wontoniii on 10/22/13.
 */
public class MFInternetService extends Service {

	//Global variables
	private static final int myID = 873182;
	private static final long REQ_TIMEOUT = 1000*30;
	private final static String APPTAG = "Drop-It";
	private String FILENAME = "dropped_messages.data";
	public static final int SEND_REQUEST = 1;
	public static final int REMOVE_REQUEST = 3;
	public static final int DROP_REQUEST_WI = 5;
	public static final int GET_HELD = 6;
	public static final int PUBLISH_HELD = 7;
	public static final int PUBLISH_MSG = 8;
	public static final int MSG_REGISTER_CLIENT = 9;
	public static final int MSG_UNREGISTER_CLIENT = 10;
	public static final int REMOTE_REQUEST = 11;
	public static final int REMOTE_DROP = 12;
	public static final int PUBLISH_MSG_SET = 13;
	public static final int UPDATE_LOCATION = 14;
	public static final int RESTART_MFSOCKET = 15;
	public static final int OTHER = 100;

	//status variables
	private static boolean isRunning = false;
	private NotificationManager nm;
	int problems;

	//Retrieval variables
	static ArrayList<DropMessage> heldMessagesList;
	static ArrayList<DropMessage> retrvMessages;
	//static ArrayList<HttpPost> remoteDropped;
	static ArrayList<GUID> sg;
	GUID lastRequest;
	boolean retrieving;
	long latestRequest;

	JMFAPI mfsocket;
	JGNRS gnrs;

	//IPC variables
	final IncomingHandler mHandler = new IncomingHandler();
	final Messenger mMessenger = new Messenger(mHandler);
	MessageRetriever mMessageRetriever;
	private Thread backgroundThread;
	private MessageRetriever backgroundRunnable;
	Messenger mClient;
	boolean binded;
    static Random randomID;


	class IncomingHandler extends Handler { // Handler of incoming messages from clients.
		@Override
		public void handleMessage(Message msg) {
			try{
				Log.d(APPTAG, "New message to service of type "+msg.what);

				switch (msg.what) {
					case MSG_REGISTER_CLIENT:
						mClient = msg.replyTo;
						binded = true;
						if(!mfsocket.isOpen()){
							try {
								mfsocket.jmfopen("basic");
							} catch (JMFException e) {
								Log.e(APPTAG, e.toString());
							}
						}
						break;
					case MSG_UNREGISTER_CLIENT:
						binded = false;
						mClient = null;
						break;
					case SEND_REQUEST:
						GUID mGUID = new GUID(msg.arg1);
						GUID locGUID = new GUID(msg.arg2);
						Log.d(APPTAG, "Send request for dropped messages at "+locGUID.getGUID()+" while latest was "+lastRequest.getGUID());
						if(lastRequest.getGUID() == locGUID.getGUID() && System.currentTimeMillis() - latestRequest < REQ_TIMEOUT){
							Log.d(APPTAG, "Not enough time has passed; sending cached data");
							try {
								mClient.send(Message.obtain(null, PUBLISH_MSG_SET, retrvMessages));
							} catch (RemoteException e) {
								Log.e(APPTAG, e.toString());
							}
						}
						else if(lastRequest.getGUID() != locGUID.getGUID()){
							Log.d(APPTAG, "Request for new location, sending out request");
							retrvMessages.clear();
							retrieving = true;
							lastRequest.setGUID(locGUID.getGUID());
							latestRequest = System.currentTimeMillis();
							SendRequest sendReq = new SendRequest(mGUID, locGUID, mfsocket, gnrs);
							new Thread(sendReq).start();
						}
						else{
							Log.d(APPTAG, "Same location but data might be old. Sending new request");
							try {
								latestRequest = System.currentTimeMillis();
								mClient.send(Message.obtain(null, PUBLISH_MSG_SET, retrvMessages));
							} catch (RemoteException e) {
								Log.e(APPTAG, e.toString());
							}
							finally {
								retrieving = true;
								SendRequest sendReq = new SendRequest(mGUID, locGUID, mfsocket, gnrs);
								new Thread(sendReq).start();
							}
						}
						break;
					case REMOVE_REQUEST:
						retrieving = false;
						break;
					case DROP_REQUEST_WI:
						Log.d(APPTAG, "Drop message");
						DropMessage temp = (DropMessage)msg.obj;
						Log.d(APPTAG, temp.getMessageBody()+" "+temp.getMessageHeader()+" "+temp.getLocationGUID()+" "+temp.getSrcGUID());
						heldMessagesList.add(temp);
						GUID tempGUID = new GUID(temp.getLocationGUID());
						GUID srcGUID = new GUID(temp.getSrcGUID());
						if(!inSet(tempGUID)){
							Log.d(APPTAG, "Add new location GUID");
							sg.add(tempGUID);
							UpdateGNRS ug = new UpdateGNRS(tempGUID, srcGUID, gnrs);
							new Thread(ug).start();
						}
						else{
							Log.d(APPTAG, "GNRS already contains the mapping");
							DroppedMessage updateLocation = new DroppedMessage(new GUID(temp.getSrcGUID()), new GUID(temp.getLocationGUID()));
							new Thread(updateLocation).start();
						}
						break;
					case GET_HELD:
						Log.d(APPTAG, "Get held messages");
						try {
							mClient.send(Message.obtain(null, PUBLISH_HELD, heldMessagesList));
						} catch (RemoteException e) {
							Log.e(APPTAG, e.toString());
						} catch (Exception e){
							Log.e(APPTAG, e.toString());
						}
						break;
					case REMOTE_REQUEST:
						Log.d(APPTAG, "Remote request");
                        int reqIdRemote = (Integer)msg.obj;
						ReplyRequest req = new ReplyRequest(new GUID(msg.arg2), new GUID(msg.arg1), mfsocket, new ArrayList<DropMessage>(heldMessagesList), reqIdRemote);
						new Thread(req).start();
						break;
					case REMOTE_DROP:
						Log.d(APPTAG, "New dropped message received");
						DropMessage newMessage = (DropMessage)msg.obj;
						if(newMessage!=null){
                            Log.d(APPTAG, "Message not null so updating the log");
							HttpPost httppost = new HttpPost(getString(com.ketonax.drop_it.R.string.stats_server));
							List<NameValuePair> nameValuePairs = new ArrayList<NameValuePair>(5);
							nameValuePairs.add(new BasicNameValuePair("Type", "RemoteDrop"));
							nameValuePairs.add(new BasicNameValuePair("srcGUID", Integer.toString(newMessage.getSrcGUID())));
							nameValuePairs.add(new BasicNameValuePair("locGUID", Integer.toString(newMessage.getLocationGUID())));
							nameValuePairs.add(new BasicNameValuePair("time", Long.toString(System.currentTimeMillis())));
                            nameValuePairs.add(new BasicNameValuePair("reqID", Integer.toString(msg.arg1)));
							httppost.setEntity(new UrlEncodedFormEntity(nameValuePairs));
							//remoteDropped.add(httppost);
							//if(remoteDropped.size()>=100){
							DroppedRemoteMessage dropMess = new DroppedRemoteMessage(httppost);
							new Thread(dropMess).start();
							//	remoteDropped.clear();
							//}
						}
						if(newMessage!=null && lastRequest.getGUID() == newMessage.getLocationGUID()){
							if(!retrvMessages.contains(newMessage)){
								retrvMessages.add(newMessage);
								if(retrieving){
									Log.d(APPTAG, "Displaying at the moment, sending to UI");
									try {
										mClient.send(Message.obtain(null, PUBLISH_MSG, newMessage));
									} catch (RemoteException e) {
										Log.e(APPTAG, e.toString());
									}
								}
								else{
									Log.d(APPTAG, "Not displaying at the moment, storing in cache");
								}
							}
							else{
								Log.d(APPTAG, "New dropped message already received");
							}
						}
						else{
							Log.d(APPTAG, "New dropped message not for the current GUID");
						}
						break;
					case UPDATE_LOCATION:
						Log.d(APPTAG, "Update location");
						UpdateLocation updateLocation = new UpdateLocation(new GUID(msg.arg1), new GUID(msg.arg2));
						new Thread(updateLocation).start();
						break;
					case RESTART_MFSOCKET:
						Log.d(APPTAG, "Restart service");
                        isRunning = false;
						mfsocket = new JMFAPI();
						try {
							mfsocket.jmfopen("basic");
						} catch (JMFException e) {
							Log.e(APPTAG, e.toString());
						}
						mMessageRetriever.stopRunning();
						backgroundThread.interrupt();
						mMessageRetriever = new MessageRetriever(mfsocket, mHandler);
						backgroundThread = new Thread(mMessageRetriever);
						backgroundThread.start();
                        heldMessagesList.clear();
                        retrvMessages.clear();
                        //remoteDropped.clear();
                        sg.clear();
                        lastRequest.setGUID(0);
                        retrieving = false;
                        latestRequest = 0;
                        gnrs.deleteGNRS();
                        String ipAddress = getLocalIpAddress();
                        if(ipAddress == null){
                            Log.e(APPTAG, "No connection available");
                            return;
                        }
                        else{
                            Log.d(APPTAG, "IP address "+ipAddress);
                        }
                        gnrs.setGNRS(getString(R.string.gnrs_IP)+":"+getString(R.string.gnrs_port), ipAddress+":"+getString(R.string.gnrs_listen_port));
                        isRunning = true;
						break;
					default:
						super.handleMessage(msg);
				}
			} catch(Exception e){
				e.printStackTrace();
			}
		}
	}


	@Override
	public void onCreate() {
		Log.d(APPTAG, "Creating service");
		super.onCreate();
		problems = 0;
		mfsocket = new JMFAPI();
		gnrs = new JGNRS();
		String ipAddress = getLocalIpAddress();
		if(ipAddress == null){
			Log.e(APPTAG, "No connection available");
			return;
		}
		else{
			Log.d(APPTAG, "IP address "+ipAddress);
		}
		gnrs.setGNRS(getString(R.string.gnrs_IP)+":"+getString(R.string.gnrs_port), ipAddress+":"+getString(R.string.gnrs_listen_port));
		isRunning = true;
		try{
			if(mfsocket == null){
				Log.e(APPTAG, "ERROR INITIALIZING MFSOCKET");
				return;
			}
			mfsocket.jmfopen("basic");
		} catch (JMFException e){
			problems = e.getCode();
		} catch (Exception e){
			Log.e(APPTAG, e.toString());
		}
		lastRequest = new GUID(0);
		retrieving = false;
		latestRequest = 0;
		heldMessagesList = new ArrayList<DropMessage>();
		loadMessages();
		retrvMessages = new ArrayList<DropMessage>();
		//remoteDropped = new ArrayList<HttpPost>();
		sg = new ArrayList<GUID>();
        randomID = new Random();
		mMessageRetriever = new MessageRetriever(mfsocket, mHandler);
		backgroundThread = new Thread(mMessageRetriever);
		backgroundThread.start();
		showNotification();
	}

	private void showNotification() {
		Intent resultIntent = new Intent(this, MainActivity.class);
		PendingIntent resultPendingIntent =
				PendingIntent.getActivity(this, 0, resultIntent, 0);
		Notification notification = new NotificationCompat.Builder(this)
				.setContentTitle("Drop it service")
				.setContentText("Running")
				.setSmallIcon(com.ketonax.drop_it.R.drawable.ic_launcher)
				.setContentIntent(resultPendingIntent)
				.build();
		startForeground(myID, notification);
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		Log.d(APPTAG, "Starting service");
		super.onStartCommand(intent, flags, startId);
		return Service.START_STICKY;
	}

	public String getLocalIpAddress() {
		try {
			return Utils.getIPAddress(true);
		} catch (Exception e){
			Log.e(APPTAG, e.toString());
		}
		return null;
	}

	private class SendRequest implements Runnable{

		GUID mGUID;
		GUID locGUID;
		JMFAPI mSocket;
		JGNRS gnrs;
        int id;

		SendRequest(GUID g, GUID locg, JMFAPI mfs, JGNRS jgnrs){
			mGUID = g;
			locGUID = locg;
			mSocket = mfs;
			gnrs = jgnrs;
            id = randomID.nextInt();
		}

		public void postData() {
			// Create a new HttpClient and Post Header
			HttpClient httpclient = new DefaultHttpClient();
			HttpPost httppost = new HttpPost(getString(com.ketonax.drop_it.R.string.stats_server));

			try {
				// Add your data
				List<NameValuePair> nameValuePairs = new ArrayList<NameValuePair>(5);
				nameValuePairs.add(new BasicNameValuePair("Type", "Request"));
				nameValuePairs.add(new BasicNameValuePair("srcGUID", Integer.toString(mGUID.getGUID())));
				nameValuePairs.add(new BasicNameValuePair("locGUID", Integer.toString(locGUID.getGUID())));
				nameValuePairs.add(new BasicNameValuePair("time", Long.toString(System.currentTimeMillis())));
                nameValuePairs.add(new BasicNameValuePair("reqID", Integer.toString(id)));
				httppost.setEntity(new UrlEncodedFormEntity(nameValuePairs));
				// Execute HTTP Post Request
				HttpResponse response = httpclient.execute(httppost);
				Log.d(APPTAG, "HTTP Post sent");

			} catch (ClientProtocolException e) {
				Log.e(APPTAG, e.toString());
			} catch (IOException e) {
				Log.e(APPTAG, e.toString());
			}
		}

		public void run(){
			try{
				String ipAddress = getLocalIpAddress();
				if(ipAddress == null){
					Log.e(APPTAG, "No connection available");
					return;
				}
				else{
					Log.d(APPTAG, "IP address "+ipAddress);
				}
				int nas[] = gnrs.lookup(locGUID.getGUID());
				Log.d(APPTAG,"GNRS lookup complete. Found " + nas.length + " bindings");
				byte buffer[];
				for(int i = 0; i<nas.length; i++){
					if(nas[i] != mGUID.getGUID()){
						buffer = NetworkMessage.genReqMessagge(locGUID, mGUID, id);
						StringBuilder sb = new StringBuilder();
						for (byte b : buffer) {
							sb.append(String.format("%02X ", b));
						}
						Log.d(APPTAG, "Sending " + sb.toString());
						int ret = mSocket.jmfsend(buffer, buffer.length, new GUID(nas[i]));
						if(ret > 0){
							Log.d(APPTAG, "Sent request to " + nas[i]);
						}
						else {
							Log.e(APPTAG, "Request for " + nas[i] + " not sent succesfully");
						}
					}
					else{
						Log.d(APPTAG, "Not sending to myself");
					}
				}
				postData();
				Log.d(APPTAG, "Send request task complete");
			} catch (JMFException e){
				Log.e(APPTAG, e.toString());
			} catch (Exception e){
				Log.e(APPTAG, e.toString());
			}
		}
	}

	private class UpdateGNRS implements Runnable{
		GUID mGUID;
		GUID na;
		JGNRS gnrs;

		UpdateGNRS(GUID g, GUID n, JGNRS jgnrs){
			mGUID = g;
			na = n;
			gnrs = jgnrs;
		}

		public void postData() {
			// Create a new HttpClient and Post Header
			HttpClient httpclient = new DefaultHttpClient();
			HttpPost httppost = new HttpPost(getString(com.ketonax.drop_it.R.string.stats_server));

			try {
				// Add your data
				List<NameValuePair> nameValuePairs = new ArrayList<NameValuePair>(4);
				nameValuePairs.add(new BasicNameValuePair("Type", "Drop"));
				nameValuePairs.add(new BasicNameValuePair("srcGUID", Integer.toString(mGUID.getGUID())));
				nameValuePairs.add(new BasicNameValuePair("locGUID", Integer.toString(na.getGUID())));
				nameValuePairs.add(new BasicNameValuePair("time", Long.toString(System.currentTimeMillis())));
				httppost.setEntity(new UrlEncodedFormEntity(nameValuePairs));
				// Execute HTTP Post Request
				HttpResponse response = httpclient.execute(httppost);
				Log.d(APPTAG, "HTTP Post sent");

			} catch (ClientProtocolException e) {
				Log.e(APPTAG, e.toString());
			} catch (IOException e) {
				Log.e(APPTAG, e.toString());
			}
		}

		public void run(){
			try{
				Log.d(APPTAG, "Update GNRS thread");
				int nas[] = new int[1];
				nas[0] = na.getGUID();
				gnrs.add(mGUID.getGUID(), nas);
				Log.d(APPTAG, "Update GNRS thread gnrs updated");
				postData();
				Log.d(APPTAG, "Update GNRS thread posted data to HTTP");
			} catch (Exception e){
				Log.e(APPTAG, e.toString());
			}
		}
	}

	private class UpdateLocation implements Runnable{
		GUID mGUID;
		GUID locGUID;

		UpdateLocation(GUID g, GUID l){
			mGUID = g;
			locGUID = l;
		}

		public void postData() {
			// Create a new HttpClient and Post Header
			HttpClient httpclient = new DefaultHttpClient();
			HttpPost httppost = new HttpPost(getString(com.ketonax.drop_it.R.string.stats_server));

			try {
				// Add your data
				List<NameValuePair> nameValuePairs = new ArrayList<NameValuePair>(4);
				nameValuePairs.add(new BasicNameValuePair("Type", "Location"));
				nameValuePairs.add(new BasicNameValuePair("srcGUID", Integer.toString(mGUID.getGUID())));
				nameValuePairs.add(new BasicNameValuePair("locGUID", Integer.toString(locGUID.getGUID())));
				nameValuePairs.add(new BasicNameValuePair("time", Long.toString(System.currentTimeMillis())));
				httppost.setEntity(new UrlEncodedFormEntity(nameValuePairs));
				// Execute HTTP Post Request
				HttpResponse response = httpclient.execute(httppost);

			} catch (ClientProtocolException e) {
				Log.e(APPTAG, e.toString());
			} catch (IOException e) {
				Log.e(APPTAG, e.toString());
			}
		}

		public void run(){
			try{
				postData();
			} catch (Exception e){
				Log.e(APPTAG, e.toString());
			}
		}
	}

	private class DroppedMessage implements Runnable{
		GUID mGUID;
		GUID locGUID;

		DroppedMessage(GUID g, GUID l){
			mGUID = g;
			locGUID = l;
		}

		public void postData() {
			// Create a new HttpClient and Post Header
			HttpClient httpclient = new DefaultHttpClient();
			HttpPost httppost = new HttpPost(getString(com.ketonax.drop_it.R.string.stats_server));

			try {
				// Add your data
				List<NameValuePair> nameValuePairs = new ArrayList<NameValuePair>(4);
				nameValuePairs.add(new BasicNameValuePair("Type", "Drop"));
				nameValuePairs.add(new BasicNameValuePair("srcGUID", Integer.toString(mGUID.getGUID())));
				nameValuePairs.add(new BasicNameValuePair("locGUID", Integer.toString(locGUID.getGUID())));
				nameValuePairs.add(new BasicNameValuePair("time", Long.toString(System.currentTimeMillis())));
				httppost.setEntity(new UrlEncodedFormEntity(nameValuePairs));
				// Execute HTTP Post Request
				HttpResponse response = httpclient.execute(httppost);

			} catch (ClientProtocolException e) {
				Log.e(APPTAG, e.toString());
			} catch (IOException e) {
				Log.e(APPTAG, e.toString());
			}
		}

		public void run(){
			try{
				postData();
			} catch (Exception e){
				Log.e(APPTAG, e.toString());
			}
		}
	}

	private class ReplyRequest implements Runnable{
		GUID destGUID;
		GUID locGUID;
		JMFAPI mSocket;
		ArrayList <DropMessage> mList;
        int reqID;

		ReplyRequest(GUID loc, GUID dest, JMFAPI sock, ArrayList <DropMessage> list, int id){
			destGUID = dest;
			locGUID = loc;
			mSocket = sock;
			mList = list;
            reqID = id;
		}

		public void run(){
			try{
				Log.d(APPTAG, "ReplyRequest start");
				byte buffer[];
				DropMessage next;
				for(Iterator<DropMessage> it = mList.iterator(); it.hasNext(); ){
					next = it.next();
					Log.d(APPTAG, "ReplyRequest iterating on message with GUID "+next.getLocationGUID());
					if(next.getLocationGUID() == locGUID.getGUID()){
						Log.d(APPTAG, "ReplyRequest sending message");
						buffer = NetworkMessage.genDropMessage(next, reqID);
						mSocket.jmfsend(buffer, buffer.length, destGUID);
					}
					else{
						Log.d(APPTAG, "ReplyRequest message with different GUID " + locGUID.getGUID() +" requested");
					}
				}
			} catch (Exception e){
				Log.e(APPTAG, e.toString());
			}
		}
	}

	private class DroppedRemoteMessage implements Runnable{

        HttpPost post;

		DroppedRemoteMessage(HttpPost p){
			post = p;
		}

		public void postData() {
			// Create a new HttpClient and Post Header
			HttpClient httpclient = new DefaultHttpClient();

			try {
				HttpResponse response = httpclient.execute(post);
                Log.d(APPTAG, "HTTP Post done");
			} catch (ClientProtocolException e) {
				Log.e(APPTAG, e.toString());
			} catch (IOException e) {
				Log.e(APPTAG, e.toString());
			}
		}

		public void run(){
			try{
				postData();
			} catch (Exception e){
				Log.e(APPTAG, e.toString());
			}
		}
	}

	private void saveMessages() {

		ObjectOutputStream outputStream = null;

		try {

			outputStream = new ObjectOutputStream(openFileOutput(FILENAME,
					Context.MODE_PRIVATE));

			outputStream.writeObject(heldMessagesList);

			outputStream.close();

		} catch (Exception e) {
			Log.e(APPTAG, e.toString());
		}

	}

	@SuppressWarnings("unchecked")
	private void loadMessages() {

		File file = getDir(FILENAME, Context.MODE_PRIVATE);

		// Exit this method if there is no such file
		if (!file.exists()) {
			return;
		}

		ObjectInputStream inputStream = null;

		try {

			inputStream = new ObjectInputStream(openFileInput(FILENAME));

			Object temp = inputStream.readObject();

			heldMessagesList = (ArrayList<DropMessage>)temp;

			inputStream.close();
		} catch (IOException e) {
			Log.e(APPTAG, e.toString());
		} catch (ClassNotFoundException e) {
			Log.e(APPTAG, e.toString());
		} catch (Exception e){
			Log.e(APPTAG, e.toString());
		}
	}

	boolean inSet(GUID guid){
		return sg.contains(guid);
	}


	@Override
	public void onDestroy() {
		Log.d(APPTAG, "Destroying service");
		super.onDestroy();
		try{
			mfsocket.jmfclose();
		} catch (JMFException e){
			problems = e.getCode();
		}
		mfsocket = null;
		isRunning = false;
		saveMessages();
	}

	public IBinder onBind(Intent intent) {
		return mMessenger.getBinder();
	}

	public static boolean isRunning() {
		return isRunning;
	}
}
