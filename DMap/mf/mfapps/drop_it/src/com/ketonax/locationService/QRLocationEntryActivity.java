package com.ketonax.locationService;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.Menu;
import android.view.View;
import android.widget.Toast;

import com.google.zxing.integration.android.IntentIntegrator;
import com.google.zxing.integration.android.IntentResult;
import com.ketonax.drop_it.R;

public class QRLocationEntryActivity extends Activity {

	static final String GEOFENCE_ID = "Geofence ID";

	private String locationGUID;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_qrlocation_entry);
		// Show the Up button in the action bar.
		setupActionBar();

		if (savedInstanceState != null)
			locationGUID = savedInstanceState.getString(GEOFENCE_ID);

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		// getMenuInflater().inflate(R.menu.qrlocation_entry, menu);
		return true;
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
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {

		IntentResult scanningResult = IntentIntegrator.parseActivityResult(
				requestCode, resultCode, data);

		if (scanningResult != null) {

			String scanContent = scanningResult.getContents();
			locationGUID = scanContent;

			Toast toast = Toast.makeText(getApplicationContext(),
					"Current location is: " + locationGUID, Toast.LENGTH_SHORT);
			toast.show();

			Intent intent = new Intent();
			intent.setData(Uri.parse(locationGUID));
			setResult(RESULT_OK, intent);

			finish();

		} else {
			Toast toast = Toast.makeText(getApplicationContext(),
					"No scan data received!", Toast.LENGTH_SHORT);
			toast.show();
		}
	}

	public void scan(View view) {

		// Start scanning
		IntentIntegrator scanIntegrator = new IntentIntegrator(this);
		scanIntegrator.initiateScan();

	}

}
