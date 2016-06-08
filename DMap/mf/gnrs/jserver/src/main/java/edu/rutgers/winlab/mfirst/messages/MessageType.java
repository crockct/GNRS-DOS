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
 * Message type identifier enumeration. Used to distinguish different network
 * message types.
 * 
 * @author Robert Moore
 */
public enum MessageType {
  /**
   * An Insert message.
   * 
   * @see InsertMessage
   */
  INSERT((byte) 0),
  /**
   * A Lookup message.
   * 
   * @see LookupMessage
   */
  LOOKUP((byte) 1),

  /**
   * An Update message.
   */
  UPDATE((byte) 2),

  /**
   * An InsertAck message.
   * 
   * @see InsertResponseMessage
   */
  INSERT_RESPONSE((byte) 0x80),
  /**
   * A LookupResponse message.
   * 
   * @see LookupResponseMessage
   */
  LOOKUP_RESPONSE((byte) 0x81),

  /**
   * An Update Response message.
   */
  UPDATE_RESPONSE((byte) 0x82),

  /**
   * An unknown message type value.
   */
  UNKNOWN((byte) 0xFF);

  /**
   * The protocol-compatible value of this message type.
   */
  private final byte value;

  /**
   * Creates a new MessageType with the specified byte value.
   * 
   * @param value
   */
  MessageType(final byte value) {
    this.value = value;
  }

  /**
   * Gets the GNRS protocol-appropriate byte for this message type.
   * 
   * @return the byte value of this message type.
   */
  public byte value() {
    return this.value;
  }

  /**
   * Parses a string and converts it to an appropriate MessageType, or the
   * UNKNOWN type if the String value is incompatible or invalid. Valid type
   * values are:
   * <ul>
   * <li>"I" for insert</li>
   * <li>"Q" for query</li>
   * <li>"A" for insert ack</li>
   * <li>"R" for query response</li>
   * <li>"U" for update</li>
   * <li>"V" for update ack</li>
   * </ul>
   * 
   * @param asString
   *          the string to parse
   * @return a MessageType based on the String, or UNKNOWN if none are
   *         appropriate.
   */
  public static MessageType parseType(final String asString) {
    MessageType type;
    if ("I".equalsIgnoreCase(asString)) {
      type = INSERT;
    } else if ("Q".equalsIgnoreCase(asString)) {
      type = LOOKUP;
    } else if ("A".equalsIgnoreCase(asString)) {
      type = INSERT_RESPONSE;
    } else if ("R".equalsIgnoreCase(asString)) {
      type = LOOKUP_RESPONSE;
    } else if ("U".equalsIgnoreCase(asString)) {
      type = UPDATE;
    } else if ("V".equalsIgnoreCase(asString)) {
      type = UPDATE_RESPONSE;
    } else {
      type = UNKNOWN;
    }
    return type;
  }
}
