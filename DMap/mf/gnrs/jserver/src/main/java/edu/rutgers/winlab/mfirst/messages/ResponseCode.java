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

/**
 * Response codes for response message.
 * 
 * @author Robert Moore
 * 
 */
public enum ResponseCode {
  /**
   * Indicates a successful action.
   */
  SUCCESS(0),
  /**
   * Indicates a general or unspecified error.
   */
  FAILED(1);
  
  /**
   * Used for toString() method.
   */
  private static final  String[] STRINGS = {"SUCCESS", "FAILED"};

  /**
   * The value of the response code for sending over the network.
   */
  private int value;

  /**
   * Creates a new ResponseCode.
   * 
   * @param value
   *          the value to send over the network.
   */
  private ResponseCode(final int value) {
    this.value = value;
  }

  /**
   * Gets this ResponseCode's value as a byte.
   * 
   * @return the byte value of this ResponseCode.
   */
  public int value() {
    return this.value;
  }
  
  /**
   * Returns a Responsecode based on the byte value.
   * @param byteValue a byte value from the GNRS network protocol.
   * @return a ResponseCode appropriate for the value.
   */
  public static ResponseCode valueOf(final int byteValue){
    return byteValue == SUCCESS.value ? SUCCESS : FAILED;
  }
  
  /**
   * Returns a String representation of this ResponseCode.
   */
  @Override
  public String toString(){
    return ResponseCode.STRINGS[this.value];
  }
}
