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
package edu.rutgers.winlab.mfirst.storage.bdb;

import com.sleepycat.persist.model.Persistent;

import edu.rutgers.winlab.mfirst.net.AddressType;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * BerkeleyDB-compatible version of a NetworkAddress
 * 
 * @author Robert Moore
 */
@Persistent
public class BDBNetworkAddress {

  /**
   * Network address type value
   */
  private int type = -1;

  /**
   * Binary value of the address
   */
  private byte[] value;

  /**
   * Converts the provided NetworkAddress to a BDBNetworkAddress.
   * 
   * @param netAddr
   *          the source NetworkAddress.
   * @return a new BDBNetworkAddress representing the NetworkAddress.
   */
  public static BDBNetworkAddress fromNetworkAddress(
      final NetworkAddress netAddr) {
    BDBNetworkAddress newAddr = null;
    if (netAddr != null) {
      newAddr = new BDBNetworkAddress();
      if (netAddr.getType() != null) {
        newAddr.type = netAddr.getType().value();
      }
      newAddr.value = netAddr.getValue();
    }

    return newAddr;
  }

  /**
   * Converts this BDBNetworkAddress to a NetworkAddress.
   * 
   * @return a NetworkAddress with the same value as this BDBNetworkAddress.
   */
  public NetworkAddress toNetworkAddress() {
    return new NetworkAddress(AddressType.valueOf(this.type), this.value);
  }

  /**
   * Returns the type value of this network address.
   * 
   * @return the type of this network address.
   */
  public int getType() {
    return this.type;
  }

  /**
   * Sets the type of this network address.
   * 
   * @param type
   *          the type of this network address.
   */
  public void setType(final int type) {
    this.type = type;
  }

  /**
   * Gets the value of this network address.
   * 
   * @return the value of this network address.
   */
  public byte[] getValue() {
    return this.value;
  }

  /**
   * Sets the value of this network address.
   * 
   * @param value
   *          the value of this network address.
   */
  public void setValue(final byte[] value) {
    this.value = value;
  }
}
