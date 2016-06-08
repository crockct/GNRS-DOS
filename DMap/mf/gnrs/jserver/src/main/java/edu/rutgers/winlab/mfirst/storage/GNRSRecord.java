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

import java.util.Collection;

import org.apache.mina.util.ConcurrentHashSet;

import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.net.NetworkAddress;

/**
 * Representation of a set of GUID bindings for GNRS servers.
 * 
 * @author Robert Moore
 * 
 */
public class GNRSRecord {
  /**
   * The GUID of this record.
   */
  private final transient GUID guid;
  /**
   * The set of bindings for the GUID.
   */
  private final transient Collection<GUIDBinding> bindings = new ConcurrentHashSet<GUIDBinding>();

  /**
   * Creates a new empty record for the specified GUID value.
   * 
   * @param guid
   *          the GUID to bind.
   */
  public GNRSRecord(final GUID guid) {
    super();
    this.guid = guid;
  }

  /**
   * Gets the GUID of this record.
   * 
   * @return the GUID for this record.
   */
  public GUID getGuid() {
    return this.guid;
  }

  /**
   * Adds a binding to this record, replacing a previous value if it is present.
   * 
   * @param binding
   *          the binding to add.
   */
  public void addBinding(final GUIDBinding binding) {
    // Remove the old binding value
    if (this.bindings.contains(binding)) {
      this.bindings.remove(binding);
    }
    this.bindings.add(binding);
  }

  /**
   * Gets the current bindings. The return value of this method may change from
   * one call to another as bindings are added/replaced/expired.
   * 
   * @return an array of current bindings for this record
   */
  public GUIDBinding[] getBindings() {
    final GUIDBinding[] addresses = new GUIDBinding[this.bindings.size()];
    int index = 0;
    for (final GUIDBinding b : this.bindings) {
      addresses[index] = b;
      ++index;
    }
    return addresses;
  }

  /**
   * Removes all bindings from this record.
   */
  public void removeAll() {
    this.bindings.clear();
  }

}
