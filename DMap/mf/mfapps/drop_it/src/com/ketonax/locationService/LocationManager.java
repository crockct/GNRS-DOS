package com.ketonax.locationService;

import edu.rutgers.winlab.jmfapi.*;

/**
 * Created by wontoniii on 10/21/13.
 */

public class LocationManager {
	public static final long REFRESH_TIME = 1000*30;
	GUID latestLocation;
	long latestUpdate;

	public LocationManager(){
		latestLocation = null;
		latestUpdate = 0;
	}

	public GUID getLatestLocation() {
		return latestLocation;
	}

	public void setLatestLocation(GUID latestLocation) {
		if(this.latestLocation == null){
			this.latestLocation = new GUID();
		}
		this.latestLocation = latestLocation;
		latestUpdate = System.currentTimeMillis();
	}

	public void setLatestLocation(int latestLocation) {
		if(this.latestLocation == null){
			this.latestLocation = new GUID();
		}
		this.latestLocation.setGUID(latestLocation);
		latestUpdate = System.currentTimeMillis();
	}

	public boolean isFresh(){
		if(System.currentTimeMillis() - latestUpdate < REFRESH_TIME){
			return true;
		}
		else{
			return false;
		}
	}
}
