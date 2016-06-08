/*
 *
 * File: JGNRS.java
 * Author: Francesco Bronzino
 *
 * Description: Java API used to interface with the GNRS library
 *
 */


package edu.rutgers.winlab.jgnrs;

/**
 * Java API used to interface with MobilityFirst's stack prototype
 */
public class JGNRS {

	private int gnrs;

	//Loading the native gnrs api and the jni wrapper libraries
	static {
		System.loadLibrary("gnrs_jni");
	}

	/*
	 * APIs implemented in the system library.
	 */
	private native int setNativeGNRS(String remote_addr, String local_addr);
	private native void freeNativeGNRS(int gnrs);
	private native int setNativeGUID(int guid);
	private native void freeNativeGUID(int guid);
	private native int[] gnrs_lookup(int gnrs, int GUID);
	private native int gnrs_add(int gnrs, int GUID, int[] NAS, int n);

	public JGNRS(){
		gnrs = 0;
	}

	public void setGNRS(String remote_addr, String local_addr){
		gnrs = setNativeGNRS(remote_addr, local_addr);
	}

	public void deleteGNRS(){
		freeNativeGNRS(gnrs);
	}

	public int[] lookup(int GUID){
		if(gnrs == 0){
			System.out.println("Set the local parameters correctly");
			return null;
		}
		int nGUID = setNativeGUID(GUID);
		int []lRes = gnrs_lookup(gnrs, nGUID);
		freeNativeGUID(nGUID);
		return lRes;
	}

	public void add(int GUID, int[] NAS){
		if(gnrs == 0){
			System.out.println("Set the local parameters correctly");
			return;
		}
		int nGUID = setNativeGUID(GUID);
		gnrs_add(gnrs, nGUID, NAS, NAS.length);
		freeNativeGUID(nGUID);
	}

}
