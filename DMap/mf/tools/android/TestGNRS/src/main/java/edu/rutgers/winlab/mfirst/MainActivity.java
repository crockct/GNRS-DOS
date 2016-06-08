package edu.rutgers.winlab.mfirst;

import android.content.Intent;
import android.os.AsyncTask;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.app.ActionBar;
import android.support.v4.app.Fragment;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.os.Build;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.RadioGroup;
import android.widget.Toast;

import java.io.IOException;
import java.util.ArrayList;

import edu.rutgers.winlab.jgnrs.JGNRS;

public class MainActivity extends ActionBarActivity {

    final String APP_TAG = "TestGNRS";
    private boolean taskRunning;
    static private AsyncTask query;
    static final ArrayList<String> retrievedGUIDs = new ArrayList<String>();
    static ArrayAdapter<String> adapterRetr;
    static ListView listView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(APP_TAG, "Getting created");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        taskRunning = false;

        adapterRetr = new ArrayAdapter<String>(
                this, android.R.layout.simple_list_item_1,
                retrievedGUIDs);

        if (savedInstanceState == null) {
            getSupportFragmentManager().beginTransaction()
                    .add(R.id.container, new GNRSLookupFragment())
                    .commit();
        }
    }

    @Override
    protected void onDestroy() {
        //cancel background task if needed
        Log.d(APP_TAG, "Getting destroyed");
        if(taskRunning){
            query.cancel(true);
        }
        super.onDestroy();
    }

    @Override
    protected void onStart() {
        Log.d(APP_TAG, "Getting started");
        super.onStart();
    }

    @Override
    protected void onStop() {
        Log.d(APP_TAG, "Getting stopped");
        super.onStop();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        Log.d(APP_TAG, "Saving instance");
        super.onSaveInstanceState(outState);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    public void startQuery(View view){
        if(!taskRunning){
            EditText text = (EditText)findViewById(R.id.editGUID);
            String GUID = text.getText().toString();
            Log.d(APP_TAG, "Selected GUID " + GUID);
            text = (EditText)findViewById(R.id.editGNRS);
            String GNRS_IP = text.getText().toString();
            Log.d(APP_TAG, "Selected GNRS " + GNRS_IP);
            text = (EditText)findViewById(R.id.editPort);
            String GNRS_PORT = text.getText().toString();
            Log.d(APP_TAG, "Selected GNRS Port " + GNRS_PORT);
            if(!GUID.isEmpty() && !GNRS_IP.isEmpty() && !GNRS_PORT.isEmpty() && Utils.isValidIP(GNRS_IP)){
                taskRunning = true;
                adapterRetr.clear();
                adapterRetr.notifyDataSetChanged();
                query = new QueryGNRS();
                ((QueryGNRS)query).execute(GNRS_IP,GNRS_PORT,GUID);
            }
            else{
                Toast.makeText(this, "Error with the given parameters", Toast.LENGTH_SHORT).show();
            }
        }
        else{
            Toast.makeText(this, "There is already a task running", Toast.LENGTH_SHORT).show();
        }
    }

    public void startAdd(View view){
        if(!taskRunning){
            EditText text = (EditText)findViewById(R.id.editGUID);
            String GUID = text.getText().toString();
            Log.d(APP_TAG, "Selected GUID to query " + GUID);
            text = (EditText)findViewById(R.id.editGUIDAdd);
            String GUIDAdd = text.getText().toString();
            Log.d(APP_TAG, "Selected GUID to add " + GUIDAdd);
            text = (EditText)findViewById(R.id.editGNRS);
            String GNRS_IP = text.getText().toString();
            Log.d(APP_TAG, "Selected GNRS " + GNRS_IP);
            text = (EditText)findViewById(R.id.editPort);
            String GNRS_PORT = text.getText().toString();
            Log.d(APP_TAG, "Selected GNRS Port " + GNRS_PORT);
            if(!GUID.isEmpty() && !GUIDAdd.isEmpty() && !GNRS_IP.isEmpty() && !GNRS_PORT.isEmpty() && Utils.isValidIP(GNRS_IP)){
                taskRunning = true;
                adapterRetr.clear();
                adapterRetr.notifyDataSetChanged();
                query = new AddGNRS();
                ((AddGNRS)query).execute(GNRS_IP,GNRS_PORT,GUID,GUIDAdd);
            }
            else{
                Toast.makeText(this, "Error with the given parameters", Toast.LENGTH_SHORT).show();
            }
        }
        else{
            Toast.makeText(this, "There is already a task running", Toast.LENGTH_SHORT).show();
        }
    }

    public void stopQuery(View view){
        if(taskRunning){
            if(!query.cancel(true)){
                Toast.makeText(this, "Task could not be stopped", Toast.LENGTH_SHORT).show();
            }
            taskRunning = false;
            adapterRetr.clear();
            adapterRetr.notifyDataSetChanged();
        }
        else{
            Toast.makeText(this, "No task running", Toast.LENGTH_SHORT).show();
        }
    }

    private void publishGUID(String GUID){
        Log.d(APP_TAG, "Adding new GUID to list");
        adapterRetr.add(GUID);
        adapterRetr.notifyDataSetChanged();
    }

    private void publishResult(Integer result){
        if(result == 0){
            Toast.makeText(this, "Query completed", Toast.LENGTH_SHORT).show();
        }
        else{
            Toast.makeText(this, "Query failed", Toast.LENGTH_SHORT).show();
        }
        taskRunning = false;
    }

    public static class GNRSLookupFragment extends Fragment {

        public GNRSLookupFragment() {
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_main, container, false);
            showListView(rootView);
            return rootView;
        }

        private void showListView(View rootView) {

            listView = (ListView) rootView.findViewById(R.id.guid_list);

            listView.setAdapter(adapterRetr);
        }
    }

    private class QueryGNRS extends AsyncTask<String, String, Integer> {

        String GnrsIp;
        String GnrsPort;
        String queryGUID;
        JGNRS gnrs;

        @Override
        protected Integer doInBackground(String... params) {
            if(params.length < 2){
                return -1;
            }
            Log.d(APP_TAG, "Query the gnrs at " + params[0]+":"+ params[1] + " for GUID " + params[2]);
            GnrsIp = params[0];
            GnrsPort = params[1];
            queryGUID = params[2];
            String ipAddress = Utils.getLocalIpAddress();
            if(ipAddress == null){
                Log.e(APP_TAG, "No connection available");
                return -1;
            }
            else{
                Log.d(APP_TAG, "IP address "+ipAddress);
            }
            gnrs = new JGNRS();
            Log.d(APP_TAG, "GNRS initiliazed");
            gnrs.setGNRS(GnrsIp+":"+GnrsPort, ipAddress+":"+"5002");
            Log.d(APP_TAG, "GNRS values set");
            int nas[] = gnrs.lookup(Integer.parseInt(queryGUID));
            Log.d(APP_TAG, "GNRS lookup completed");
            gnrs.deleteGNRS();
            gnrs = null;
            for(int i = 0; i<nas.length; i++){
                publishProgress(String.valueOf(nas[i]));
            }
            Log.d(APP_TAG,"GNRS lookup complete. Found " + nas.length + " bindings");
            return 0;
        }

        @Override
        protected void onProgressUpdate(String... values) {
            Log.d(APP_TAG, "New GUID value " + values[0]);
            publishGUID(values[0]);
        }

        @Override
        protected void onPostExecute(Integer result){
            Log.d(APP_TAG, "Finished the execution, I'm in the UI thread " + result);
            publishResult(result);
        }

        @Override
        protected void onCancelled(Integer result){
            Log.d(APP_TAG, "Cancelled with result " + result);
            gnrs.deleteGNRS();
        }

        @Override
        protected void onCancelled() {
            Log.d(APP_TAG, "Cancelled without result ");
            gnrs.deleteGNRS();
        }
    }

    private class AddGNRS extends AsyncTask<String, String, Integer> {

        String GnrsIp;
        String GnrsPort;
        String queryGUID;
        String addGUID;
        JGNRS gnrs;

        @Override
        protected Integer doInBackground(String... params) {
            if(params.length < 3){
                return -1;
            }
            Log.d(APP_TAG, "Add to the gnrs at  " + params[0]+":"+ params[1] + " for GUID " + params[2] + " adding " + params[3]);
            GnrsIp = params[0];
            GnrsPort = params[1];
            queryGUID = params[2];
            addGUID = params[3];
            String ipAddress = Utils.getLocalIpAddress();
            if(ipAddress == null){
                Log.e(APP_TAG, "No connection available");
                return -1;
            }
            else{
                Log.d(APP_TAG, "IP address "+ipAddress);
            }
            gnrs = new JGNRS();
            gnrs.setGNRS(GnrsIp+":"+GnrsPort, ipAddress+":"+"5002");
            gnrs.add(Integer.parseInt(queryGUID), new int[]{Integer.valueOf(addGUID)});
            Log.d(APP_TAG,"GNRS add complete.");
            gnrs.deleteGNRS();
            gnrs = null;
            return 0;
        }

        @Override
        protected void onProgressUpdate(String... values) {
            Log.d(APP_TAG, "New GUID value " + values[0]);
            publishGUID(values[0]);
        }

        @Override
        protected void onPostExecute(Integer result){
            Log.d(APP_TAG, "Finished the execution, I'm in the UI thread " + result);
            publishResult(result);
        }

        @Override
        protected void onCancelled(Integer result){
            Log.d(APP_TAG, "Cancelled with result " + result);
            if(gnrs!=null)
                gnrs.deleteGNRS();
        }

        @Override
        protected void onCancelled() {
            Log.d(APP_TAG, "Cancelled without result ");
            if(gnrs!=null)
                gnrs.deleteGNRS();
        }
    }
}
