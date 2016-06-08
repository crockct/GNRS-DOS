/*
 * Copyright (c) 2012, Rutgers University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * + Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * + Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
package edu.rutgers.winlab.mfirst.messages;

import java.util.LinkedList;
import java.util.List;

import edu.rutgers.winlab.mfirst.messages.opt.Option;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * Common fields for all GNRS application messages.
 * 
 * @author Robert Moore
 * 
 */

public abstract class AbstractMessage {

  /**
   * The time at which this object was created, in nanosecond. Note that
   * nanosecond time is only relative and has nothing to do with "wall time".
   */
  public final long createdNanos = System.nanoTime();
  
  /**
   * How long this message was queued (created until received by worker).
   */
  public long queueNanos = 0l;
  
  /**
   * How much time is spent processing the message.
   */
  public long processingNanos = 0l;
  
  /**
   * Nanosecond timestamp of when the request was forwarded to a remote server.
   */
  public long forwardNanos = 0l;

  /**
   * Actually used as 32-bit unsigned int
   */
  protected long requestId;

  /**
   * Version number of the protocol format.
   */
  protected byte version = 0;

  /**
   * Unsigned 8-bit type value
   */
  protected MessageType type;

  /**
   * Who sent the message originally.
   */
  protected NetworkAddress originAddress;
  
  /**
   * List of message options
   */
  protected List<Option> options = new LinkedList<Option>();

  /**
   * Protected constructor. Only to be called by subclasses.
   */
  protected AbstractMessage() {
    super();
  }

  /**
   * Gets the request ID of this message. Note that the wire protocol specifies
   * an unsigned 32-bit integer, but Java doesn't support unsigned types.
   * 
   * @return the request ID of this message.
   */
  public long getRequestId() {
    return this.requestId;
  }

  /**
   * Gets the type of this message. Depends on the subclass of this message.
   * 
   * @return the type value of this message.
   * @see MessageType
   */
  public MessageType getType() {
    return this.type;
  }

  /**
   * Gets the NetworkAddress of the originator of this message.
   * 
   * @return the NetworkAddress of the message originator.
   */
  public NetworkAddress getOriginAddress() {
    return this.originAddress;
  }

  /**
   * Sets the request ID for this message. The protocol specifies an unsigned
   * 32-bit integer value, but Java does not support unsigned types.
   * 
   * @param requestId
   */
  public void setRequestId(final long requestId) {
    this.requestId = (requestId & 0xFFFFFFFFl);
  }

  /**
   * Sets the type of this message. Should only be called by subclasses.
   * 
   * @param type
   *          the type of this message.
   * @see MessageType
   */
  protected void setType(final MessageType type) {
    this.type = type;
  }

  /**
   * Sets the originator address for this message.
   * 
   * @param address
   *          the originator address for this message.
   */
  public void setOriginAddress(final NetworkAddress address) {
    this.originAddress = address;
  }

  /**
   * The total encoded length of this message, in bytes.
   * 
   * @return the length of this message, in bytes, when encoded according to the
   *         network protocol.
   */
  public int getMessageLength() {
    // (Version, type, length)->4, (request id)->4, (options offset, payload offset) -> 4
    int length = 12 + this.getPayloadLength();
    // requestor address
    if(this.originAddress != null){
      // Origin address type, origin address length
      length += 4; 
      length += this.originAddress.getLength();
    }
    if(!this.options.isEmpty()){
      for(Option opt : this.options){
        length += 2+opt.getLength();
      }
    }
    
    return length;
  }

  /**
   * The length of the "payload" section of this message.
   * 
   * @return the length of the payload of this message.
   */
  protected abstract int getPayloadLength();

  /**
   * The version value of this message.
   * 
   * @return the version of this message.
   */
  public byte getVersion() {
    return this.version;
  }

  /**
   * Sets the version value of this message.
   * 
   * @param version
   *          the new version value.
   */
  public void setVersion(final byte version) {
    this.version = version;
  }
  
  /**
   * Adds an option to this message.
   * @param option
   */
  public void addOption(final Option option){
    this.options.add(option);
  }
  
  /**
   * Gets the list of options for this message.
   * @return
   */
  public List<Option> getOptions(){
    return this.options;
  }
  
  public void finalizeOptions(){
    if(!this.options.isEmpty()){
      Option lastOption = this.options.get(this.options.size()-1);
      lastOption.setFinal();
    }
  }
}
