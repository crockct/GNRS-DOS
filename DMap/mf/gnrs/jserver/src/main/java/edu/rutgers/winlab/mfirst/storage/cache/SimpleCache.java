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
package edu.rutgers.winlab.mfirst.storage.cache;

import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.rutgers.winlab.mfirst.GUID;
import edu.rutgers.winlab.mfirst.storage.GUIDBinding;

/**
 * A very simple cache using an in-memory LRU structure.
 * 
 * @author Robert Moore
 */
public class SimpleCache {

  /**
   * Logging for this cache.
   */
  private static final Logger LOG = LoggerFactory.getLogger(SimpleCache.class);

  /**
   * The actual cache used.
   */
  private final transient LRUCache<GUID, CacheEntry> cache;

  /**
   * Creates a new cache for the specified number of GUID entries.
   * 
   * @param size
   *          the size of the cache.
   */
  public SimpleCache(final int size) {
    super();
    this.cache = new LRUCache<GUID, CacheEntry>(size);
  }

  /**
   * Puts a set of bindings into this cache. Replaces any previous bindings in
   * the cache.
   * 
   * @param guid
   *          the GUID to bind.
   * @param bindings
   *          the new bindings.
   */
  public void put(final GUID guid, final CacheOrigin origin,
      final GUIDBinding... bindings) {

    final Set<GUIDBinding> bindSet = new HashSet<GUIDBinding>();
    for (GUIDBinding b : bindings) {
      bindSet.add(b);
    }
    final CacheEntry entry = new CacheEntry(bindSet, origin);

    this.cache.put(guid, entry);
  }

  /**
   * Get the bindings for a GUID from the cache.
   * 
   * @param guid
   *          the GUID to retrieve
   * @return the bindings (if any) for the GUID stored in the cache.
   */
  public Collection<GUIDBinding> get(final GUID guid) {
    CacheEntry entry = this.cache.get(guid);

    Collection<GUIDBinding> bindings = null;
    if (entry != null) {
      bindings = entry.getBindings();
      // Verify TTL and expiration values and remove the entries if they are too
      // old
      long now = System.currentTimeMillis();

      if (bindings != null) {

        for (Iterator<GUIDBinding> iter = bindings.iterator(); iter.hasNext();) {
          GUIDBinding bind = iter.next();
          if (bind.getExpiration() < now || bind.getTtl() < now) {
            iter.remove();
          }
        }
        if (bindings.isEmpty()) {
          this.cache.remove(guid);
          bindings = null;
        }
      }
    }

    return bindings;
  }
}
