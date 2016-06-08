/**
 *
 * File: GUID.java
 * Author: Francesco Bronzino
 *
 * Description: GUID class, Representation of a GUID
 *
 */

package edu.rutgers.winlab.jmfapi;

public class GUID {

	private int mGuidVal;

	public GUID(){
		mGuidVal = 0;
	}

	public GUID(int val){
		mGuidVal = val;
	}

	public int getGUID(){
		return mGuidVal;
	}

	public void setGUID(int val){
		mGuidVal = val;
	}

	@Override
	public boolean equals(Object o) {
		GUID other = (GUID)o;
		return mGuidVal == other.getGUID();
	}
}
