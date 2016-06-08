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

import java.util.Arrays;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Network Address class for GNRS. Represents a network endpoint for
 * communication with or identification of other hosts or content in the
 * network.
 * 
 * @author Robert Moore
 */
public class NetworkAddress {

  /**
   * Logging for this class.
   */
  @SuppressWarnings("unused")
  private static final Logger LOG = LoggerFactory
      .getLogger(NetworkAddress.class);

  /**
   * The type of address.
   */
  protected AddressType type;

  /**
   * Raw (binary) form of this network address. Depends on the type of address.
   */
  protected byte[] value;

  /**
   * Creates a new NetworkAddress object with the specified type and value
   * 
   * @param type
   *          the type of the address
   * @param value
   *          the binary value of the address
   */
  public NetworkAddress(final AddressType type, final byte[] value) {
    super();
    if (value != null && value.length > type.getMaxLength()) {
      throw new IllegalArgumentException(
          "NetworkAddress value length is greater than max allowed by " + type);
    }
    this.setType(type);
    this.setValue(value);

  }

  /**
   * Returns the raw (binary) form of this network address as a byte array.
   * 
   * @return this network address as a byte array.
   */
  public byte[] getValue() {
    return this.value;
  }

  /**
   * Sets the new value of this network address from a byte array.
   * 
   * @param bytes
   *          the new value of this network address.
   */
  public final void setValue(final byte[] bytes) {
    if (bytes != null && bytes.length > 0xFFFF) {
      throw new IllegalArgumentException(
          "NetworkAddress value exceeds maximum length of 65535 bytes.");
    }
    this.value = bytes;
  }

  @Override
  public String toString() {
    final StringBuilder sBuff = new StringBuilder(this.getLength() * 2 + 4);
    sBuff.append("NA(");

    for (final byte b : this.value) {
      sBuff.append(String.format("%02x", Byte.valueOf(b)));
    }
    sBuff.append(')');
    return sBuff.toString();
  }

  @Override
  public int hashCode() {
    return Arrays.hashCode(this.value);
  }

  @Override
  public boolean equals(final Object other) {
    boolean equals;
    if (other instanceof NetworkAddress) {
      equals = this.equalsNA((NetworkAddress) other);
    } else {
      equals = super.equals(other);
    }
    return equals;
  }

  /**
   * Determines if this NetworkAddress equals another based on their type and
   * value.
   * 
   * @param address
   *          another NetworkAddress
   * @return {@code true} if they are equal.
   */
  public boolean equalsNA(final NetworkAddress address) {
    boolean isEqual;
    if (address == null) {
      isEqual = false;
    } else {
      if (this.type == null) {
        if (address.type == null) {
          isEqual = true;
        } else {
          isEqual = false;
        }
      } else {
        isEqual = this.type.equals(address.type);
      }
      if (isEqual) {
        isEqual = Arrays.equals(this.value, address.value);
      }
    }
    return isEqual;
  }

  /**
   * Returns the type of address this represents.
   * 
   * @return the address type.
   */
  public AddressType getType() {
    return this.type;
  }

  /**
   * Sets the type for this address.
   * 
   * @param type
   *          the new type.
   */
  public final void setType(final AddressType type) {
    this.type = type;
  }

  /**
   * Returns the length of the value of this address.
   * 
   * @return the length of this address in bytes.
   */
  public int getLength() {
    return this.value == null ? 0 : this.value.length;
  }

}
