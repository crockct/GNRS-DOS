package com.ketonax.locationService;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.app.NavUtils;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

import com.ketonax.drop_it.R;

public class ManualLocationEntryActivity extends Activity {

	static final String GEOFENCE_ID = "Geofence ID";

	String locationGUID;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_manual_location_entry);
		// Show the Up button in the action bar.
		setupActionBar();

		if (savedInstanceState != null)
			locationGUID = savedInstanceState.getString(GEOFENCE_ID);

	}

	@Override
	protected void onSaveInstanceState(Bundle outState) {
		// TODO Auto-generated method stub
		
		outState.putString(GEOFENCE_ID, locationGUID);
		super.onSaveInstanceState(outState);
	}

	/**
	 * Set up the {@link android.app.ActionBar}.
	 */
	private void setupActionBar() {

		getActionBar().setDisplayHomeAsUpEnabled(true);

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case android.R.id.home:
			// This ID represents the Home or Up button. In the case of this
			// activity, the Up button is shown. Use NavUtils to allow users
			// to navigate up one level in the application structure. For
			// more details, see the Navigation pattern on Android Design:
			//
			// http://developer.android.com/design/patterns/navigation.html#up-vs-back
			//
			NavUtils.navigateUpFromSameTask(this);
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	public void select(View view) {

		EditText editText = (EditText) findViewById(R.id.location_field_entry);
		locationGUID = editText.getText().toString();

		if (locationGUID.isEmpty())
			Toast.makeText(this, "Please enter a location.", Toast.LENGTH_LONG)
					.show();
		else {

			Intent data = new Intent();
			data.setData(Uri.parse(locationGUID));
			setResult(RESULT_OK, data);

			finish();

		}

	}

}
