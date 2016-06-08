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
package edu.rutgers.winlab.mfirst.net;

/**
 * Enum representing the different types of network address values possible.
 * Contains both the {@code type} field and the maximum value of the
 * {@code length} field for each type.
 * 
 * @author Robert Moore
 * 
 */
public enum AddressType {
  /**
   * Represents an Internet Protocol (IP) version 4 address plus a UDP port.
   */
  INET_4_UDP(0, 6),
  
  /**
   * Represents a GUID value.
   */
  GUID(1,20);

  /**
   * Strings for toString() method.
   */
  private static final String[] STRINGS = { "IPv4+UDP", "GUID" };

  /**
   * Unsigned short representing the type.
   */
  private final int type;
  /**
   * Maximum length of the value, specific to each type.
   */
  private final  int maxLength;

  /**
   * Private constructor.
   * 
   * @param type
   *          the type of address.
   * @param maxLength
   *          the maximum length of an address value.
   */
  private AddressType(final int type, final int maxLength) {
    this.type = type;
    this.maxLength = maxLength;
  }

  /**
   * Converts the provided unsigned short into an AddressType or {@code null} if
   * the type is not unrecognized.
   * 
   * @param asInt
   *          the unsigned short representing the type.
   * @return an AddressType for the value, or {@code null} if none can be found
   *         that matches.
   */
  public static AddressType valueOf(final int asInt) {
    AddressType type = null;
   
    if (asInt == INET_4_UDP.type) {
      type = INET_4_UDP;
    }else if(asInt == GUID.type){
      type = GUID;
    }
    return type;
  }

  @Override
  public String toString() {
    return AddressType.STRINGS[this.type];
  }

  /**
   * Returns this address type value as a short.
   * 
   * @return the short value of this type.
   */
  public int value() {
    return this.type;
  }

  /**
   * Returns the maximum length of this address type.
   * 
   * @return the maximum length of this address type.
   */
  public int getMaxLength() {
    return this.maxLength;
  }
}
