/**
 *
 * File: JMFAPI.java
 * Author: Francesco Bronzino
 *
 * Description: Java API used to interface with MobilityFirst's stack prototype
 *
 */


package edu.rutgers.winlab.jmfapi;

/**
 * Java API used to interface with MobilityFirst's stack prototype
 *
 */
public class JMFAPI {

	/**
	 * MobilityFirst socket object creation. Cannot perform any functionality until open is called.
	 */
	public JMFAPI() {
		handle = 0;
	}

	//Class variables
	private int handle;
	
	
	//Loading the native library that includes the jni wrapper
	static {
		System.loadLibrary("mfapi_jni");
	}

	/*
	 * JNI API implemented in the system library.
	 */
	private native int mfopen(String profile, int opts, int GUID) throws CException;
	private native int mfsend(int handle, byte[] data, int size, int dst_GUID, int opts) throws CException;
	private native int mfrecv(int handle, GUID sGUID, byte[] data, int size, int []src_GUID, int nGUID) throws CException;
	private native int mfrecv_blk(int handle, GUID sGUID, byte[] data, int size, int []src_GUID, int nGUID) throws CException;
	private native int mfattach(int handle, int []GUIDs, int nGUID) throws CException;
	private native int mfdetach(int handle, int []GUIDs, int nGUID) throws CException;
	private native int mfclose(int handle) throws CException;


	/**
	 * Open the socket connection with the stack
	 * @param profile At the moment contains the string representing the destination GUID
	 * @throws JMFException if an error occurs during the open operation.
	 *          Usually is due to an error at the API level and in that case contains the errno value
	 */
	public void jmfopen(String profile) throws JMFException{
		if(handle != 0){
			throw new JMFException("Trying to open an already open socket");
		}
		//calls the native method mfopen
		//if everything goes well it stores the communication socket
		try {
			handle = mfopen(profile, 0, 0);
		} catch(CException e){
			throw new JMFException(e.getMessage(), e.getCause());
		}
	}

	/**
	 * Open the socket connection with the stack
	 * @param profile At the moment contains the string representing the destination GUID
	 * @param opts Options parameter; can NOT be null
	 * @throws JMFException if an error occurs during the open operation.
	 *          Usually is due to an error at the API level and in that case contains the errno value
	 */
	public void jmfopen(String profile, MFFlag opts) throws JMFException{
		if(handle != 0){
			throw new JMFException("Trying to open an already open socket");
		}
		//calls the native method mfopen
		//if everything goes well it stores the communication socket
		try {
			handle = mfopen(profile, opts.getValue(), 0);
		} catch(CException e){
			throw new JMFException(e.getMessage(), e.getCause());
		}
	}

	/**
	 * Open the socket connection with the stack
	 * @param profile At the moment contains the string representing the destination GUID
	 * @param guid Contains the source GUID
	 * @throws JMFException if an error occurs during the open operation.
	 *          Usually is due to an error at the API level and in that case contains the errno value
	 */
	public void jmfopen(String profile, GUID guid) throws JMFException{
		if(handle != 0){
			throw new JMFException("Trying to open an already open socket");
		}
		//calls the native method mfopen
		//if everything goes well it stores the communication socket
		try {
			handle = mfopen(profile, 0, guid.getGUID());
		} catch(CException e){
			throw new JMFException(e.getMessage(), e.getCause());
		}
	}

	/**
	 * Open the socket connection with the stack
	 * @param profile At the moment contains the string representing the destination GUID
	 * @param guid Contains the source GUID
	 * @param opts Options parameter; can NOT be null
	 * @throws JMFException if an error occurs during the open operation.
	 *          Usually is due to an error at the API level and in that case contains the errno value
	 */
	public void jmfopen(String profile, MFFlag opts, GUID guid) throws JMFException{
		if(handle != 0){
			throw new JMFException("Trying to open an already open socket");
		}
		//calls the native method mfopen
		//if everything goes well it stores the communication socket
		try {
			handle = mfopen(profile, opts.getValue(), guid.getGUID());
		} catch(CException e){
			throw new JMFException(e.getMessage(), e.getCause());
		}
	}

	/**
	 * Send a chunk of data. The chunk can not be larger than 1 MB
	 * @param data data to be sent
	 * @param size size of the chunk
	 * @param dst_GUID destination GUID for the packet
	 * @param opts Options parameter; can NOT be null
	 * @return returns the number of bytes sent (it should correspond to size)
	 * @throws JMFException if an error occurs during the send operation
	 *          Usually is due to an error at the API level and in that case contains the errno value
	 */
	public int jmfsend(byte[] data, int size, GUID dst_GUID, MFFlag opts) throws JMFException{
		if(handle == 0){
			throw new JMFException("Trying to execute an operation without opening the socket session");
		}
		int ret = -1;
		try {
			ret = mfsend(handle, data, size, dst_GUID.getGUID(), opts.getValue());
		} catch(CException e) {
            throw new JMFException(e.getMessage(), e.getCause());
		}
		if(ret<0){
			throw new JMFException();
		}
		return ret;
	}

	/**
	 * Send a chunk of data. The chunk can not be larger than 1 MB
	 * @param data data to be sent
	 * @param size size of the chunk
	 * @param dst_GUID destination GUID for the packet
	 * @return returns the number of bytes sent (it should correspond to size)
	 * @throws JMFException if an error occurs during the send operation
	 *          Usually is due to an error at the API level and in that case contains the errno value
	 */
	public int jmfsend(byte[] data, int size, GUID dst_GUID) throws JMFException{
		if(handle == 0){
			throw new JMFException("Trying to execute an operation without opening the socket session");
		}
		int ret = -1;
		try {
			ret = mfsend(handle, data, size, dst_GUID.getGUID(), 0);
		} catch(CException e) {
			throw new JMFException(e.getMessage(), e.getCause());
		}
		if(ret<0){
			throw new JMFException();
		}
		return ret;
	}

    /*
    TODO: To properly support the sGUID writing it should be important to move to a GUID class instead of using only integers.
     */
	/**
	 * Tries to receive the last message received. If no message has been received it will return immediately without waiting.
     * @param sGUID GUID of the sender. Is written if not null
	 * @param data buffer to be used for the received data
	 * @param size max number of bytes that should be received
	 * @param src_GUID set of GUIDs to filter receive
	 * @return the number of bytes written into the buffer, -1 if there was no message to receive.
	 * @throws JMFException if an error occurs during the recv operation
	 *          Usually is due to an error at the API level and in that case contains the errno value
	 */
	public int jmfrecv(GUID sGUID, byte[] data, int size, GUID []src_GUID) throws JMFException{
		if(handle == 0){
			throw new JMFException("Trying to execute an operation without opening the socket session");
		}
		int ret = -1;
		try {
			int []srcg = new int[src_GUID.length];
			for(int i = 0; i<src_GUID.length; i++){
				srcg[i] = src_GUID[i].getGUID();
			}
			ret = mfrecv(handle, sGUID, data, size, srcg, srcg.length);
		} catch(Exception e) {
            throw new JMFException(e.getMessage(), e.getCause());
		}
		if(ret<0){
			throw new JMFException();
		}
        return ret;
	}

	/**
	 * Tries to receive the last message received. If no message has been received it will return immediately without waiting.
     * @param sGUID GUID of the sender. Is written if not null
	 * @param data buffer to be used for the received data
	 * @param size max number of bytes that should be received
	 * @return the number of bytes written into the buffer, -1 if there was no message to receive.
	 * @throws JMFException if an error occurs during the recv operation
	 *          Usually is due to an error at the API level and in that case contains the errno value
	 */
	public int jmfrecv(GUID sGUID, byte[] data, int size) throws JMFException{
		if(handle == 0){
			throw new JMFException("Trying to execute an operation without opening the socket session");
		}
		int ret = -1;
		try {
			ret = mfrecv(handle, sGUID, data, size, null, 0);
		} catch(Exception e) {
			throw new JMFException(e.getMessage(), e.getCause());
		}
		if(ret<0){
			throw new JMFException();
		}
		return ret;
	}

	/**
	 * Tries to receive the last message received. If no message has been received it blocks until one will be retrieved.
     * @param sGUID GUID of the sender. Is written if not null
	 * @param data buffer to be used for the received data
	 * @param size max number of bytes that should be received
	 * @param src_GUID set of GUIDs to filter receive
	 * @return the number of bytes written into the buffer.
	 * @throws JMFException if an error occurs during the recv operation
	 *          Usually is due to an error at the API level and in that case contains the errno value
	 */
	public int jmfrecv_blk(GUID sGUID, byte[] data, int size, GUID []src_GUID) throws JMFException{
		if(handle == 0){
			throw new JMFException("Trying to execute an operation without opening the socket session");
		}
		int ret = -1;
		try {
			int []srcg = new int[src_GUID.length];
			for(int i = 0; i<src_GUID.length; i++){
				srcg[i] = src_GUID[i].getGUID();
			}
			ret = mfrecv_blk(handle, sGUID, data, size, srcg, srcg.length);
		} catch(Exception e) {
			throw new JMFException(e.getMessage(), e.getCause());
		}
		if(ret<0){
			throw new JMFException();
		}
		return ret;
	}

	/**
	 * Tries to receive the last message received. If no message has been received it blocks until one will be retrieved.
     * @param sGUID GUID of the sender. Is written if not null
	 * @param data buffer to be used for the received data
	 * @param size max number of bytes that should be received
	 * @return the number of bytes written into the buffer.
	 * @throws JMFException if an error occurs during the recv operation
	 *          Usually is due to an error at the API level and in that case contains the errno value
	 */
	public int jmfrecv_blk(GUID sGUID, byte[] data, int size) throws JMFException{

		if(handle == 0){
			throw new JMFException("Trying to execute an operation without opening the socket session");
		}
		int ret = -1;
		try {
			ret = mfrecv_blk(handle, sGUID, data, size, null, 0);
		} catch(Exception e) {
			throw new JMFException(e.getMessage(), e.getCause());
		}
		if(ret<0){
			throw new JMFException();
		}
		return ret;
	}

	/**
	 * Add GUID reachability for the specified GUIDs.
	 * @param GUIDs set of GUIDs to add to socket end point
	 * @throws JMFException if an error occurs during the attach operation
	 *          Usually happens if the stack was not able to add reachability
	 */
	public void jmfattach(GUID []GUIDs) throws JMFException{
		if(handle == 0){
			throw new JMFException("Trying to execute an operation without opening the socket session");
		}
		int ret = -1;
		try {
			int []srcg = new int[GUIDs.length];
			for(int i = 0; i<GUIDs.length; i++){
				srcg[i] = GUIDs[i].getGUID();
			}
			ret = mfattach(handle, srcg, srcg.length);
		} catch(CException e) {
			throw new JMFException(e.getMessage(), e.getCause());
		}
		if(ret<0){
			throw new JMFException();
		}
	}

	/**
	 * Add GUID reachability for the specified GUIDs.
	 * @param guid GUID to add to socket end point
	 * @throws JMFException if an error occurs during the attach operation
	 *          Usually happens if the stack was not able to add reachability
	 */
	public void jmfattach(GUID guid) throws JMFException{
		if(handle == 0){
			throw new JMFException("Trying to execute an operation without opening the socket session");
		}
		int ret = -1;
		int GUIDs[] = new int[1];
		GUIDs[0] = guid.getGUID();
		try {
			ret = mfattach(handle, GUIDs, 1);
		} catch(CException e) {
			throw new JMFException(e.getMessage(), e.getCause());
		}
		if(ret<0){
			throw new JMFException();
		}
	}

	/**
	 * Remove GUID reachability for the specified GUIDs.
	 * @param GUIDs set of GUIDs to remove to socket end point
	 * @throws JMFException if an error occurs during the detach operation
	 *          Usually happens if the stack was not able to remove reachability
	 */
	public void jmfdetach(GUID []GUIDs) throws JMFException{
		if(handle == 0){
			throw new JMFException("Trying to execute an operation without opening the socket session");
		}
		int ret = -1;
		try {
			int []srcg = new int[GUIDs.length];
			for(int i = 0; i<GUIDs.length; i++){
				srcg[i] = GUIDs[i].getGUID();
			}
			ret = mfdetach(handle, srcg, GUIDs.length);
		} catch(CException e) {
			throw new JMFException(e.getMessage(), e.getCause());
		}
		if(ret<0){
			throw new JMFException();
		}
	}

	/**
	 * Remove GUID reachability for the specified GUID.
	 * @param guid GUID to remove to socket end point
	 * @throws JMFException if an error occurs during the detach operation
	 *          Usually happens if the stack was not able to remove reachability
	 */
	public void jmfdetach(GUID guid) throws JMFException{
		if(handle == 0){
			throw new JMFException("Trying to execute an operation without opening the socket session");
		}
		int ret = -1;
		int GUIDs[] = new int[1];
		GUIDs[0] = guid.getGUID();
		try {
			ret = mfdetach(handle, GUIDs, 1);
		} catch(CException e) {
			throw new JMFException(e.getMessage(), e.getCause());
		}
		if(ret<0){
			throw new JMFException();
		}
	}

	/**
	 * Close the socket connection with the stack.
	 * @throws JMFException if an error occurs during the close operation
	 *          Usually is due to an error at the API level and in that case contains the errno value
	 */
	public void jmfclose() throws JMFException{
		if(handle == 0){
			throw new JMFException("Trying to execute an operation without opening the socket session");
		}
		//If everything goes well the socked id is set to 0
		try {
			mfclose(handle);
			handle = 0;
		} catch(CException e){
			throw new JMFException(e.getMessage(), e.getCause());
		}
	}

	/**
	 * Check if the the stack is open (done locally, no connection to stack).
	 */
	public boolean isOpen(){
		return handle != 0;
	}
}
