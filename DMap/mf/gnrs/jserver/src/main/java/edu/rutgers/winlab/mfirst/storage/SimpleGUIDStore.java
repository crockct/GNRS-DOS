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

import java.util.concurrent.ConcurrentHashMap;

import edu.rutgers.winlab.mfirst.GUID;

/**
 * A simple in-memory GUID storage engine with no persistence after the
 * application terminates.
 * <p>
 * The implementation is completely threadsafe in the sense that it will not
 * throw a {@code ConcurrentModificationException} if it is access
 * simultaneously by multiple threads. However, the bindings it stores may NOT
 * be consistent for multi-threaded insert/get operations.
 * </p>
 * 
 * @author Robert Moore
 */
public class SimpleGUIDStore implements GUIDStore {

  /**
   * Backing map.
   */
  private final ConcurrentHashMap<GUID, GNRSRecord> storageMap = new ConcurrentHashMap<GUID, GNRSRecord>();

  @Override
  public GNRSRecord getBindings(final GUID guid) {
    return this.storageMap.get(guid);
  }

  @Override
  public boolean appendBindings(final GUID guid, final GUIDBinding... bindings) {
    GNRSRecord currRecord = this.storageMap.get(guid);
    if (currRecord == null) {
   // First-time insert, so synch
      synchronized (this.storageMap) {
        // Extra work, but only once per record
        currRecord = this.storageMap.get(guid);
        if (currRecord == null) {
          currRecord = new GNRSRecord(guid);
          this.storageMap.put(guid, currRecord);
        }
      }
    }

    if (bindings != null) {
      for (final GUIDBinding b : bindings) {
        currRecord.addBinding(b);
      }
    }

    return true;
  }

  @Override
  public void doInit() {
    // Nothing to do
  }

  @Override
  public void doShutdown() {
    this.storageMap.clear();
  }

  @Override
  public boolean replaceBindings(final GUID guid, final GUIDBinding... bindings) {
    GNRSRecord currRecord = this.storageMap.get(guid);
    if (currRecord == null) {
      // First-time insert, so synch
      synchronized (this.storageMap) {
        // Extra work, but only once per record
        currRecord = this.storageMap.get(guid);
        if (currRecord == null) {
          currRecord = new GNRSRecord(guid);
          this.storageMap.put(guid, currRecord);
        }
      }
    }
    //Remove previous values
    currRecord.removeAll();

    if (bindings != null) {
      for (final GUIDBinding b : bindings) {
        currRecord.addBinding(b);
      }
    }

    return true;
  }

}
