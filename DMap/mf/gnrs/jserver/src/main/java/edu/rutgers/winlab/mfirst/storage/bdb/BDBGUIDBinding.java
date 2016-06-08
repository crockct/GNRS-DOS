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

import edu.rutgers.winlab.mfirst.net.NetworkAddress;
import edu.rutgers.winlab.mfirst.storage.GUIDBinding;

/**
 * BerkeleyDB-compatible version of GUID binding.
 * 
 * @author Robert Moore
 */
@Persistent
public class BDBGUIDBinding {

  /**
   * The network address for this binding
   */
  public BDBNetworkAddress address;

  /**
   * The TTL value for this binding.
   */
  public long ttl;

  /**
   * The weight or priority of this binding. Used to provide preference to
   * different bindings when a client requests.
   */
  public int weight;

  /**
   * The expiration time of this binding.
   */
  public long expiration;

  /**
   * Creates a new, empty binding.
   */
  public BDBGUIDBinding() {
    super();
  }

  /**
   * Creates a BDB GUID binding from a GUIDBinding object.
   * 
   * @param binding
   *          the source GUID binding.
   * @return a new BDB GUID binding.
   */
  public static BDBGUIDBinding fromGUIDBinding(final GUIDBinding binding) {

    final BDBGUIDBinding returnBind = new BDBGUIDBinding();

    returnBind.address = BDBNetworkAddress.fromNetworkAddress(binding
        .getAddress());
    returnBind.ttl = binding.getTtl();
    returnBind.weight = binding.getWeight();
    returnBind.expiration = binding.getExpiration();

    return returnBind;
  }

  /**
   * Converts this BDBGUIDBinding to a GUIDBinding object.
   * 
   * @return a new GUIDBinding object with the same information as this binding.
   */
  public GUIDBinding toGUIDBinding() {
    final GUIDBinding binding = new GUIDBinding();
    if (this.address == null) {
//      binding.setAddress(new NetworkAddress(null,null));
    }else{
      binding.setAddress(this.address.toNetworkAddress());
    }
    binding.setTtl(this.ttl);
    binding.setWeight(this.weight);
    binding.setExpiration(expiration);
    return binding;
  }
}
