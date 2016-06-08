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
package edu.rutgers.winlab.mfirst.storage;

import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * A GNRS record entry for a single Network Address binding value.
 * 
 * @author Robert Moore
 */
public class GUIDBinding {

  /**
   * The network address for this binding
   */
  private NetworkAddress address;

  /**
   * The TTL value for this binding.
   */
  private long ttl;

  /**
   * The weight or priority of this binding. Used to provide preference to
   * different bindings when a client requests.
   */
  private int weight;

  /**
   * When the binding is no longer valid.
   */
  private long expiration;

  /**
   * Get the address that this binding holds.
   * 
   * @return the network address that this binding holds.
   */
  public NetworkAddress getAddress() {
    return this.address;
  }

  /**
   * Sets the new network address binding.
   * 
   * @param address
   *          the new network address for this binding.
   */
  public void setAddress(final NetworkAddress address) {
    this.address = address;
  }

  /**
   * Time To Live value of this binding. Note that the wire protocol specifies
   * and unsigned 32-bit integer, but Java does not support unsigned types.
   * 
   * @return the TTL value.
   */
  public long getTtl() {
    return this.ttl;
  }

  /**
   * Sets the new TTL value for this binding. Note that the wire protocol
   * specifies an unsigned 64-bit integer, but Java does not support unsigned
   * types, so signed is used to approximate.
   * 
   * @param ttl
   *          the new TTL value.
   */
  // FIXME: Need to handle 64-bit unsigned type
  public void setTtl(final long ttl) {
    this.ttl = ttl;
  }

  /**
   * Weight or preference value for this binding. Note that the wire protocol
   * specifies an unsigned 16-bit value, but Java does not support unsigned
   * types.
   * 
   * @return the weight of this binding.
   */
  public int getWeight() {
    return this.weight;
  }

  /**
   * Sets the weight value for this binding. Note that the wire protocol
   * specifies an unsigned 16-bit value, but Java does not support unsigned
   * types.
   * 
   * @param weight
   *          the new weight value.
   */
  public void setWeight(final int weight) {
    this.weight = weight & 0xFFFF;
  }

  @Override
  public int hashCode() {
    return this.address.hashCode();
  }

  @Override
  public boolean equals(final Object other) {
    boolean equal;
    if (other instanceof GUIDBinding) {
      equal = this.equalsBinding((GUIDBinding) other);
    } else {
      equal = super.equals(other);
    }
    return equal;
  }

  /**
   * Determines equality of this GUIDBinding and another based on the values of
   * their NetworkAddresses.
   * 
   * @param binding
   *          the other GUIDBinding.
   * @return {@code true} if and only if {@code this.address.equals(b.address)}
   */
  public boolean equalsBinding(final GUIDBinding binding) {
    boolean isEqual;

    if (this.address == null) {
      if (binding.address == null) {
        isEqual = true;
      } else {
        isEqual = false;
      }

    } else {
      isEqual = this.address.equalsNA(binding.address);
    }
    return isEqual;
  }

  @Override
  public String toString() {
    final StringBuilder sBuff = new StringBuilder();
    sBuff.append("Bind (").append(this.address).append(", T").append(this.ttl)
        .append("/E").append(this.expiration).append(", ").append(this.weight)
        .append(")");
    return sBuff.toString();

  }

  public long getExpiration() {
    return expiration;
  }

  public void setExpiration(long expiration) {
    this.expiration = expiration;
  }

}
