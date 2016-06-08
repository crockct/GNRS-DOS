package com.ketonax.drop_it;


import android.util.Log;

/**
 * Created by wontoniii on 10/25/13.
 */
public class Utils {
	final static String p1WifiMac = "50:a4:c8:ea:b6:90";
	final static String p1WimaxMac = "84:51:81:16:1e:82";
	final static int p1GUID = 275468;
	final static String p2WifiMac = "9c:3a:af:6b:ed:b2";
	final static String p2WimaxMac = "50:01:bb:83:6e:a2";
	final static int p2GUID = 339459;
	final static String p3WifiMac = "9c:3a:af:6b:07:b9";
	final static String p3WimaxMac = "50:01:bb:83:63:5d";
	final static int p3GUID = 275468;
	final static String p4WifiMac = "6c:f3:73:99:fc:34";
	final static String p4WimaxMac = "temp";
	final static int p4GUID = 527582;
	final static String p5WifiMac = "6c:f3:73:99:fc:ac";
	final static String p5WimaxMac = "temp";
	final static int p5GUID = 704028;
	final static String p6WifiMac = "6c:f3:73:99:fb:ac";
	final static String p6WimaxMac = "temp";
	final static int p6GUID = 339459;
	final static String p7WifiMac = "9c:3a:af:78:fa:15";
	final static String p7WimaxMac = "00:00:00:00:00:00";
	final static int p7GUID = 801588;
	final static String p8WifiMac = "38:e7:d8:6e:d5:e6";
	final static String p8WimaxMac = "00:18:41:87:44:34";
	final static int p8GUID = 2462;
	final static String p9WifiMac = "64:a7:69:93:5f:2f";
	final static String p9WimaxMac = "64:a7:69:05:33:f8";
	final static int p9GUID = 77433;
	final static String p10WifiMac = "64:a7:69:93:5d:0f";
	final static String p10WimaxMac = "64:a7:69:05:33:8a";
	final static int p10GUID = 951692;
    final static String p11WifiMac = "6c:f3:73:99:fe:76";
    final static String p11WimaxMac = "00:00:00:00:00:00";
    final static int p11GUID = 50;

	public static int getPhoneGUID(){
		int guid= -1;
		String wifiMac = com.ketonax.networkLogic.Utils.getMACAddress("wlan0");
		String wimaxMac = com.ketonax.networkLogic.Utils.getMACAddress("wimax0");
		String wimaxMacAlt = com.ketonax.networkLogic.Utils.getMACAddress("uwbr0");
		Log.d("Drop-It", "Values found "+wifiMac+" "+wimaxMac+" "+wimaxMacAlt);
		if(wimaxMac.equals("") && wimaxMac.equals("") && wifiMac.equals("")){
			Log.d("Drop-It", "No valid values found");
			guid = -1;
		}
		else if (!wifiMac.equals("")){
            Log.d("Drop-It", "Has wifi so checking");
			if(wifiMac.toLowerCase().equals(p1WifiMac)) guid = p1GUID;
			else if(wifiMac.toLowerCase().equals(p2WifiMac.toLowerCase())) guid = p2GUID;
			else if(wifiMac.toLowerCase().equals(p3WifiMac.toLowerCase())) guid = p3GUID;
			else if(wifiMac.toLowerCase().equals(p4WifiMac.toLowerCase())) guid = p4GUID;
			else if(wifiMac.toLowerCase().equals(p5WifiMac.toLowerCase())) guid = p5GUID;
			else if(wifiMac.toLowerCase().equals(p6WifiMac.toLowerCase())) guid = p6GUID;
			else if(wifiMac.toLowerCase().equals(p7WifiMac.toLowerCase())) guid = p7GUID;
			else if(wifiMac.toLowerCase().equals(p8WifiMac.toLowerCase())) guid = p8GUID;
			else if(wifiMac.toLowerCase().equals(p9WifiMac.toLowerCase())) guid = p9GUID;
			else if(wifiMac.toLowerCase().equals(p10WifiMac.toLowerCase())) guid = p10GUID;
            else if(wifiMac.toLowerCase().equals(p11WifiMac.toLowerCase())) guid = p11GUID;
			else guid = -1;
		}
		else if (!wimaxMac.equals("")){
            Log.d("Drop-It", "Has wimax so checking");
			if(wimaxMac.toLowerCase().equals(p1WimaxMac.toLowerCase())) guid = p1GUID;
			else if(wimaxMac.toLowerCase().equals(p2WimaxMac.toLowerCase())) guid = p2GUID;
			else if(wimaxMac.toLowerCase().equals(p3WimaxMac.toLowerCase())) guid = p3GUID;
			else if(wimaxMac.toLowerCase().equals(p4WimaxMac.toLowerCase())) guid = p4GUID;
			else if(wimaxMac.toLowerCase().equals(p5WimaxMac.toLowerCase())) guid = p5GUID;
			else if(wimaxMac.toLowerCase().equals(p6WimaxMac.toLowerCase())) guid = p6GUID;
			else if(wimaxMac.toLowerCase().equals(p7WimaxMac.toLowerCase())) guid = p7GUID;
			else if(wimaxMac.toLowerCase().equals(p8WimaxMac.toLowerCase())) guid = p8GUID;
			else if(wimaxMac.toLowerCase().equals(p9WimaxMac.toLowerCase())) guid = p9GUID;
			else if(wimaxMac.toLowerCase().equals(p10WimaxMac.toLowerCase())) guid = p10GUID;
            else if(wimaxMac.toLowerCase().equals(p11WimaxMac.toLowerCase())) guid = p11GUID;
			else guid = -1;
		}
		else if (!wimaxMacAlt.equals("")){
            Log.d("Drop-It", "Has wimaxAlt so checking");
			if(wimaxMacAlt.toLowerCase().equals(p1WimaxMac.toLowerCase())) guid = p1GUID;
			else if(wimaxMacAlt.toLowerCase().equals(p2WimaxMac.toLowerCase())) guid = p2GUID;
			else if(wimaxMacAlt.toLowerCase().equals(p3WimaxMac.toLowerCase())) guid = p3GUID;
			else if(wimaxMacAlt.toLowerCase().equals(p4WimaxMac.toLowerCase())) guid = p4GUID;
			else if(wimaxMacAlt.toLowerCase().equals(p5WimaxMac.toLowerCase())) guid = p5GUID;
			else if(wimaxMacAlt.toLowerCase().equals(p6WimaxMac.toLowerCase())) guid = p6GUID;
			else if(wimaxMacAlt.toLowerCase().equals(p7WimaxMac.toLowerCase())) guid = p7GUID;
			else if(wimaxMacAlt.toLowerCase().equals(p8WimaxMac.toLowerCase())) guid = p8GUID;
			else if(wimaxMacAlt.toLowerCase().equals(p9WimaxMac.toLowerCase())) guid = p9GUID;
			else if(wimaxMacAlt.toLowerCase().equals(p10WimaxMac.toLowerCase())) guid = p10GUID;
            else if(wimaxMacAlt.toLowerCase().equals(p11WimaxMac.toLowerCase())) guid = p11GUID;
			else guid = -1;
		}
		return guid;
	}
}
