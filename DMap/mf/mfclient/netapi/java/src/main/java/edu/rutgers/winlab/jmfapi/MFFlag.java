/**
 *
 * File: JMFAPI.java
 * Author: Francesco Bronzino
 *
 * Description: MobilityFirst flags used to request features and service Ids to the host protocol stack
 *
 */

package edu.rutgers.winlab.jmfapi;

/**
 * Object that provides support functions to express flags in inter process communications with the stack
 *
 */
public class MFFlag {

    /**
     * Multicast flag
     */
    static public int MF_MULTICAST= 0x00000001;
    /**
     * Anycast flag
     */
    static public int MF_ANYCAST = 0x00000002;
    /**
     * Multihoming flag
     */
    static public int MF_MULTIHOME =  0x00000004;
    /**
     * Content request flag
     */
    static public int MF_CONTENT_REQUEST= 0x00000200;
    /**
     * Content response flag
     */
    static public int MF_CONTENT_RESPONSE= 0x00000400;
    /**
     * Broadcast flag
     */
    static public int MF_BROADCAST = 0x00002000;

    private int value;

    public int getValue() {
        return value;
    }

    public void setValue(int value) {
        this.value = value;
    }

    public boolean isMFFlagSet(int f){
        return (value & f) != 0;
    }

    public void setMFFlag(int f){
        value = value | f;
    }

    public boolean isMFFlagSet(MFFlag f){
        return (value & f.getValue()) != 0;
    }

    public void setMFFlag(MFFlag f){
        value = value | f.getValue();
    }

    public void setMultihoming(){
        value = value | MF_MULTIHOME;
    }

    public boolean isMultihomingSet(){
        return (value & MF_MULTIHOME) != 0;
    }

    public void setAnycast(){
        value = value | MF_ANYCAST;
    }

    public boolean isAnycast(){
        return (value & MF_ANYCAST) != 0;
    }

    public void setMulticast() {
        value = value | MF_MULTICAST;
    }

    public boolean isMulticast(){
        return (value & MF_MULTICAST) != 0;
    }

    public void setBroadcast(){
        value = value | MF_BROADCAST;
    }

    public boolean isBroadcast(){
        return (value & MF_BROADCAST) != 0;
    }

}
