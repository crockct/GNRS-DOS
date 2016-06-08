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
package edu.rutgers.winlab.mfirst;

import java.util.Set;

import org.apache.mina.util.ConcurrentHashSet;

import edu.rutgers.winlab.mfirst.messages.AbstractMessage;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * @author Robert Moore
 *
 */
public class RelayInfo {

  /**
   * The original client message.
   */
  public AbstractMessage clientMessage;
  
  /**
   * The addresses for the response (if a LookupResponse).
   */
  public Set<NetworkAddress> responseAddresses = new ConcurrentHashSet<NetworkAddress>();
  
  /**
   * The servers that have yet to respond.
   */
  public final Set<NetworkAddress> remainingServers = new ConcurrentHashSet<NetworkAddress>();
  
  /**
   * Number of attempts made so far.
   */
  private transient int numAttempts = 0;
  
  /**
   * Timestamp of the last attempt to send the message.
   */
  private transient long lastAttempt = 0l;
  
  /**
   * Marks this info to indicate that an attempt was made.
   * @return returns the current number of attempts (including this one).
   */
  public int markAttempt(){
    
    this.lastAttempt = System.currentTimeMillis();
    return ++this.numAttempts;
  }

  /**
   * Get the number of attempts made.
   * @return the number of attempts made.
   * @see #markAttempt()
   */
  public int getNumAttempts() {
    return this.numAttempts;
  }

  /**
   * Get the timestamp (in milliseconds since the Unix epoch) of the last
   * attempt.
   * @return the timestamp of the last attempt, or 0 if no attempt has previously been made.
   * @see #markAttempt()
   */
  public long getAttemptTs() {
    return this.lastAttempt;
  }
  
}
