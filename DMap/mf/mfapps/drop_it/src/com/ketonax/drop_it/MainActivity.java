/*Broadcast: ACTION_MESSAGE_DROPPED*/
/*Filename: dropped_messages.data*/
/*Use retrievedMessagesList to put any messages retrieved from the location*/
package com.ketonax.drop_it;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.Locale;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.ActionBar;
import android.app.FragmentTransaction;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;

import com.ketonax.locationService.LocationManager;
import com.ketonax.locationService.ManualLocationEntryActivity;
import com.ketonax.locationService.QRLocationEntryActivity;
import com.ketonax.message_package.DropMessage;
import com.ketonax.networkLogic.MFInternetService;

public class MainActivity extends FragmentActivity implements ActionBar.TabListener {

	/* Global Variables */
	private final static int CONNECTION_FAILURE_RESOLUTION_REQUEST = 1000;
	private final static int LOCATION_CLARIFICATION_REQUEST = 2000;
	private final static int MANUAL_LOCATION_REQUEST = 3000;
	private final static int QR_LOCATION_REQUEST = 4000;
	public final static String APPTAG = "Drop-It";

	private final static String ACTION_MESSAGE_DROPPED = "com.ketonax.drop_it.MESSAGE_DROPPED";

	// Flag that indicates if a request is underway.
	private boolean requestInProgress;

	// Defines the allowable request types.
	public enum REQUEST_TYPE {
		ADD, REMOVE_INTENT
	}

	private REQUEST_TYPE requestType;

	final static int phoneGUID = Utils.getPhoneGUID(); // Must be changed if installing on multiple


	private static final String MESSAGE_HEADER = "message head";
	private static final String MESSAGE_BODY = "message body";
	private final static String MESSAGE_ID = "message id";
	private static final String MESSAGE_LIST = "message list";

	private static final String REQUEST_IN_PROGRESS = "request in progress";

	// DropMessage object to store new messages
	DropMessage newMessage = null;

	static ListView listView;

	static final ArrayList<DropMessage> heldMessagesList = new ArrayList<DropMessage>();
	static ArrayAdapter<DropMessage> adapterHeld;
	static final ArrayList<DropMessage> retrievedMessagesList = new ArrayList<DropMessage>();
	static ArrayAdapter<DropMessage> adapterRetr;

	private EditText messageTitleView;
	private EditText messageTextView;

	private static LocationManager mLocationManager;

	static Messenger mService = null;
	boolean mIsBound;
	final Messenger mMessenger = new Messenger(new IncomingHandler());

	class IncomingHandler extends Handler {
		@Override
		public void handleMessage(Message msg) {
			Log.d(APPTAG, "New message from service type " + msg.what);
			try{
				switch (msg.what) {
					case MFInternetService.PUBLISH_HELD:
						if(previousPosition == 2){
							Log.d(APPTAG, "List of held messages");
							@SuppressWarnings("unchecked")
							ArrayList <DropMessage> temp = (ArrayList <DropMessage>)msg.obj;
							Log.d(APPTAG, "List of held messages has size " + temp.size());
							for(Iterator<DropMessage> it = temp.iterator(); it.hasNext(); ){
								adapterHeld.add(it.next());
							}
							adapterHeld.notifyDataSetChanged();
						}
						break;
					case MFInternetService.PUBLISH_MSG:
						DropMessage tempMessage = (DropMessage)msg.obj;
						if(tempMessage != null && previousPosition == 1
								&& tempMessage.getLocationGUID()==mLocationManager.getLatestLocation().getGUID()){
							Log.d(APPTAG, "Publish new message");
							adapterRetr.add(tempMessage);
							adapterRetr.notifyDataSetChanged();
						}
						break;
					case MFInternetService.PUBLISH_MSG_SET:
						Log.d(APPTAG, "Publish new messages");
						@SuppressWarnings("unchecked")
						ArrayList <DropMessage> tempRetr = (ArrayList <DropMessage>)msg.obj;
						if(tempRetr != null && previousPosition == 1
								&& tempRetr.size()>0 &&
								tempRetr.get(0).getLocationGUID()==mLocationManager.getLatestLocation().getGUID()){
							Log.d(APPTAG, "List of held messages has size " + tempRetr.size());
							for(Iterator<DropMessage> it = tempRetr.iterator(); it.hasNext(); ){
								adapterRetr.add(it.next());
							}
							adapterRetr.notifyDataSetChanged();
						}
						break;
					default:
						super.handleMessage(msg);
				}
			} catch (Exception e){
				Log.e(APPTAG, e.toString());
			}
		}
	}

	private ServiceConnection mConnection = new ServiceConnection() {
		public void onServiceConnected(ComponentName className, IBinder service) {
			Log.d(APPTAG, "The service has been connected");
			mService = new Messenger(service);
			try {
				Message msg = Message.obtain(null, MFInternetService.MSG_REGISTER_CLIENT);
				msg.replyTo = mMessenger;
				mService.send(msg);
			} catch (RemoteException e) {
				// In this case the service has crashed before we could even do anything with it
				Log.e(APPTAG, "The service has crashed before we could do anything");
			}
		}

		public void onServiceDisconnected(ComponentName className) {
			// This is called when the connection with the service has been unexpectedly disconnected - process crashed.
			Log.d(APPTAG, "The service got disconnected");
			mService = null;
		}
	};

	/**
	 * The {@link android.support.v4.view.PagerAdapter} that will provide
	 * fragments for each of the sections. We use a
	 * {@link android.support.v4.app.FragmentPagerAdapter} derivative, which
	 * will keep every loaded fragment in memory. If this becomes too memory
	 * intensive, it may be best to switch to a
	 * {@link android.support.v4.app.FragmentStatePagerAdapter}.
	 */
	SectionsPagerAdapter mSectionsPagerAdapter;

	/**
	 * The {@link ViewPager} that will host the section contents.
	 */
	ViewPager mViewPager;

	int previousPosition;

	@SuppressWarnings("unchecked")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		// Set up the action bar.
		final ActionBar actionBar = getActionBar();
		actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);

		// Create the adapter that will return a fragment for each of the
		// primary sections of the app.
		mSectionsPagerAdapter = new SectionsPagerAdapter(
				getSupportFragmentManager());

		// Set up the ViewPager with the sections adapter.
		mViewPager = (ViewPager) findViewById(R.id.pager);
		mViewPager.setAdapter(mSectionsPagerAdapter);

		// When swiping between different sections, select the corresponding
		// tab. We can also use ActionBar.Tab#select() to do this if we have
		// a reference to the Tab.
		mViewPager
				.setOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener() {
					@Override
					public void onPageSelected(int position) {
						actionBar.setSelectedNavigationItem(position);
					}
				});

		// For each of the sections in the app, add a tab to the action bar.
		for (int i = 0; i < mSectionsPagerAdapter.getCount(); i++) {
			// Create a tab with text corresponding to the page title defined by
			// the adapter. Also specify this Activity object, which implements
			// the TabListener interface, as the callback (listener) for when
			// this tab is selected.
			actionBar.addTab(actionBar.newTab()
					.setText(mSectionsPagerAdapter.getPageTitle(i))
					.setTabListener(this));
		}
		// Start with the request flag set to false
		requestInProgress = false;

		CheckIfServiceIsRunning();

		mLocationManager = new LocationManager();

		previousPosition = 0;

		if (savedInstanceState != null) {
			requestInProgress = savedInstanceState
					.getBoolean(REQUEST_IN_PROGRESS);
			previousPosition = savedInstanceState.getInt("LAST_FRAGMENT", 0);
		}

		adapterHeld = new ArrayAdapter<DropMessage>(
				this, android.R.layout.simple_list_item_1,
				heldMessagesList);

		adapterRetr = new ArrayAdapter<DropMessage>(
				this, android.R.layout.simple_list_item_1,
				retrievedMessagesList);

		Log.d(APPTAG, "I am phone " + phoneGUID);
	}

	private void CheckIfServiceIsRunning() {
		//If the service is running when the activity starts, we want to automatically bind to it.
		if (MFInternetService.isRunning()) {
			doBindService();
		}
		else {
			Intent intent = new Intent(this, MFInternetService.class);
			startService(intent);
			doBindService();
		}
	}

	private void doBindService() {
		Log.d(APPTAG, "Binding to service");
		bindService(new Intent(this, MFInternetService.class), mConnection, Context.BIND_AUTO_CREATE);
		mIsBound = true;
	}

	private void doUnbindService() {
		Log.d(APPTAG, "Unbinding to service");
		if (mIsBound) {
			// If we have received the service, and hence registered with it, then now is the time to unregister.
			if (mService != null) {
				try {
					Message msg = Message.obtain(null, MFInternetService.MSG_UNREGISTER_CLIENT);
					msg.replyTo = mMessenger;
					mService.send(msg);
				} catch (RemoteException e) {
					// There is nothing special we need to do if the service has crashed.
				}
			}
			// Detach our existing connection.
			unbindService(mConnection);
			mIsBound = false;
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.main, menu);
		return super.onCreateOptionsMenu(menu);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// TODO Auto-generated method stub

		switch (item.getItemId()) {
		case R.id.manual_loc_entry:
			Intent intent = new Intent(this, ManualLocationEntryActivity.class);
			startActivityForResult(intent, MANUAL_LOCATION_REQUEST);
			return true;
		case R.id.qr_loc_entry:
			Intent intent2 = new Intent(this, QRLocationEntryActivity.class);
			startActivityForResult(intent2, QR_LOCATION_REQUEST);
			return true;
		case R.id.restart_service:
			try {
				mService.send(Message.obtain(null, MFInternetService.RESTART_MFSOCKET));
			} catch (RemoteException e) {
				Log.e(APPTAG, e.toString());
			}
			return true;
			default:
			return super.onOptionsItemSelected(item);

		}
	}

	@Override
	protected void onSaveInstanceState(Bundle outState) {
		outState.putBoolean(REQUEST_IN_PROGRESS, requestInProgress);
		outState.putInt("LAST_FRAGMENT", previousPosition);

		super.onSaveInstanceState(outState);

	}

	@Override
	protected void onStart() {
		super.onStart();

		Log.d(APPTAG, "Phone GUID = " + phoneGUID);
		doBindService();

	}

	@Override
	protected void onResume() {
		super.onResume();
	}

	@Override
	protected void onPause() {
		/*
		LocalBroadcastManager.getInstance(this).unregisterReceiver(
				messageReceiver);
		*/

		super.onPause();

	}

	@Override
	protected void onStop() {

		// Stop Async Tasks
		//TODO change this to the communication with the service
		stopMessageDownload();
		//stopMessageUpload();
		doUnbindService();
		super.onStop();
	}

	@Override
	protected void onDestroy() {
		try {
			doUnbindService();
		} catch (Throwable t) {
			Log.e("MainActivity", "Failed to unbind from the service", t);
		}
		super.onDestroy();
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode,
			Intent intent) {

		switch (requestCode) {
		case MANUAL_LOCATION_REQUEST:
			switch(resultCode){

			case RESULT_OK:
				Log.d(APPTAG, "Requesting manual location input from user");
				String tempS = intent.getDataString();
				mLocationManager.setLatestLocation(Integer.parseInt(tempS));
			}
			try {
				mService.send(Message.obtain(null, MFInternetService.UPDATE_LOCATION));
			} catch (RemoteException e) {
				Log.e(APPTAG, e.toString());
			}
			break;
		case QR_LOCATION_REQUEST:
			switch (resultCode) {

			case RESULT_OK:
				Log.d(APPTAG, "Requesting QR location scan");
				String test = intent.getDataString();
				try {
					JSONObject jObj = new JSONObject(test);
					String fence = jObj.getString("HRN");
					String GUID = jObj.getString("GUID");
					Log.d(APPTAG, "Requesting QR location scan result: " + fence + " " + GUID);
					mLocationManager.setLatestLocation(Integer.parseInt(GUID));
				} catch (JSONException e) {
					Log.e(APPTAG, e.toString());
				} catch (Exception e) {
					Log.e(APPTAG, e.toString());
				}
				try {
					mService.send(Message.obtain(null, MFInternetService.UPDATE_LOCATION, phoneGUID, mLocationManager.getLatestLocation().getGUID()));
				} catch (RemoteException e) {
					Log.e(APPTAG, e.toString());
				}
			}
			break;

		default:
			Log.d(APPTAG, "Unkown activity request code");
			break;
		}
	}

	@Override
	public void onTabSelected(ActionBar.Tab tab,
			FragmentTransaction fragmentTransaction) {
		// When the given tab is selected, switch to the corresponding page in
		// the ViewPager.
		if (tab.getPosition() == 0){
			if(previousPosition == 1) stopMessageDownload();
			previousPosition = 0;
		}
		else if (tab.getPosition() == 1){
			if(previousPosition != 1) startMessageDownload();
			previousPosition = 1;
		}
		else{
			if(previousPosition == 1) stopMessageDownload();
			previousPosition = 2;
			updateHeldMessages();
		}

		mViewPager.setCurrentItem(tab.getPosition());

	}

	@Override
	public void onTabUnselected(ActionBar.Tab tab,
			FragmentTransaction fragmentTransaction) {
	}

	@Override
	public void onTabReselected(ActionBar.Tab tab,
			FragmentTransaction fragmentTransaction) {
	}

	/**
	 * A {@link FragmentPagerAdapter} that returns a fragment corresponding to
	 * one of the sections/tabs/pages.
	 */
	public class SectionsPagerAdapter extends FragmentPagerAdapter {

		public SectionsPagerAdapter(FragmentManager fm) {
			super(fm);
		}

		@Override
		public Fragment getItem(int position) {

			Fragment fragment = null;

			if (position == 0)
				fragment = new DropMessageFragment();
			else if (position == 1)
				fragment = new GetMessagesFragment();
			else
				fragment = new heldMessagesFragment();

			return fragment;

		}

		@Override
		public int getCount() {
			// Show 3 total pages.
			return 3;
		}

		@Override
		public CharSequence getPageTitle(int position) {
			Locale l = Locale.getDefault();
			switch (position) {
			case 0:
				return getString(R.string.title_section1).toUpperCase(l);
			case 1:
				return getString(R.string.title_section2).toUpperCase(l);
			case 2:
				return getString(R.string.title_section3).toUpperCase(l);

			}
			return null;
		}
	}

	public static class DropMessageFragment extends Fragment {

		public DropMessageFragment() {
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
				Bundle savedInstanceState) {

			View rootView = inflater.inflate(R.layout.fragment_drop_message,
					container, false);

			return rootView;
		}

	}

	public static class GetMessagesFragment extends Fragment {

		View rootView;

		public GetMessagesFragment() {
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
				Bundle savedInstanceState) {

			rootView = inflater.inflate(R.layout.fragment_get_messages,
					container, false);

			showListView(rootView);

			return rootView;

		}

		private void showListView(View rootView) {

			listView = (ListView) rootView.findViewById(R.id.messages_list);

			listView.setAdapter(adapterRetr);

			listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {

				@Override
				public void onItemClick(AdapterView<?> parent, View view,
						int position, long id) {

					if (retrievedMessagesList.get(position).containsMessage()) {

						Intent intent = new Intent(getActivity(),
								DisplayMessageActivity.class);

						Bundle extras = new Bundle();
						extras.putString(MESSAGE_HEADER, retrievedMessagesList
								.get(position).getMessageHeader());

						extras.putString(MESSAGE_BODY, retrievedMessagesList
								.get(position).getMessageBody());

						intent.putExtras(extras);

						startActivity(intent);

					} else
						Toast.makeText(getActivity(), "Unkown message type!",
								Toast.LENGTH_SHORT).show();

				}
			});
		}
	}

	public static class heldMessagesFragment extends Fragment {

		View rootView;

		public heldMessagesFragment() {
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container,
				Bundle savedInstanceState) {

			rootView = inflater.inflate(R.layout.fragment_get_messages,
					container, false);

			showListView(rootView);

			return rootView;

		}

		private void showListView(View rootView) {

			listView = (ListView) rootView.findViewById(R.id.messages_list);

			listView.setAdapter(adapterHeld);

			listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {

				@Override
				public void onItemClick(AdapterView<?> parent, View view,
						int position, long id) {

					if (heldMessagesList.get(position).containsMessage()) {

						Intent intent = new Intent(getActivity(),
								DisplayMessageActivity.class);

						Bundle extras = new Bundle();
						extras.putString(MESSAGE_HEADER,
								heldMessagesList.get(position)
										.getMessageHeader());

						extras.putString(MESSAGE_BODY,
								heldMessagesList.get(position).getMessageBody());

						intent.putExtras(extras);

						startActivity(intent);

					} else
						Toast.makeText(getActivity(), "Unkown message type!",
								Toast.LENGTH_SHORT).show();

				}
			});
		}
	}

	// TODO Create a getGUID(String geofenceID)

	public void dropIt(View view) {

		messageTitleView = (EditText) findViewById(R.id.message_title_area);
		messageTextView = (EditText) findViewById(R.id.message_text_area);

		String messageHeader = messageTitleView.getText().toString();
		String messageBody = messageTextView.getText().toString();

		if (mLocationManager.getLatestLocation() == null || !mLocationManager.isFresh()) {

			Toast.makeText(this, "Can't post. Unknown or old location.",
					Toast.LENGTH_LONG).show();

		} else if (messageHeader.isEmpty())

			Toast.makeText(this, "Title field is blank!", Toast.LENGTH_SHORT)
					.show();
		else if (messageBody.isEmpty())
			Toast.makeText(this, "Message field is blank!", Toast.LENGTH_SHORT)
					.show();
		else {

			newMessage = new DropMessage(messageHeader, messageBody,
					mLocationManager.getLatestLocation().getGUID());
			newMessage.setSrcGUID(phoneGUID);

			try {
				mService.send(Message.obtain(null, MFInternetService.DROP_REQUEST_WI, newMessage));
				Toast.makeText(this,
						"Your message has been saved under " + mLocationManager.getLatestLocation().getGUID(),
						Toast.LENGTH_LONG).show();

				// Clear the EditText Views
				messageTitleView.setText("");
				messageTextView.setText("");

				Intent broadcastIntent = new Intent();
				broadcastIntent.setAction(ACTION_MESSAGE_DROPPED).putExtra(
						MESSAGE_ID, newMessage.getMessageID());
			} catch (RemoteException e) {
				Log.e(APPTAG, e.toString());
				Toast.makeText(this,
						"Error communicating with MF service",
						Toast.LENGTH_LONG).show();
			}

		}

	}


	public void startMessageDownload() {

		Log.d(APPTAG, "Message download started.");

		adapterRetr.clear();
		adapterRetr.notifyDataSetChanged();
		try {
			int tempGUID = 0;
			if(mLocationManager.getLatestLocation() != null){
				tempGUID = mLocationManager.getLatestLocation().getGUID();
			}
			if(mService != null){
				mService.send(Message.obtain(null, MFInternetService.SEND_REQUEST, phoneGUID, tempGUID));
			}
			else{
				Log.d(APPTAG, "The service is not binded!");
			}
		} catch (RemoteException e) {
			Log.e(APPTAG, e.toString());
		}

	}

	public void stopMessageDownload() {
		try {
			mService.send(Message.obtain(null, MFInternetService.REMOVE_REQUEST));
		} catch (RemoteException e) {
			Log.e(APPTAG, e.toString());
		}
		Log.d(APPTAG, "Message download stopped.");
	}

	public void stopMessageUpload() {

		Log.d(APPTAG, "Message upload stopped.");
	}

	public void updateHeldMessages(){
		adapterHeld.clear();
		adapterHeld.notifyDataSetChanged();
		try {
			mService.send(Message.obtain(null, MFInternetService.GET_HELD));
		} catch (RemoteException e) {
			Log.e(APPTAG, e.toString());
		}
	}

}
