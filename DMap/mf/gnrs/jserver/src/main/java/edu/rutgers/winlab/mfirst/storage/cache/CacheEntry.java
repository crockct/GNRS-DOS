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
import java.util.LinkedList;

import edu.rutgers.winlab.mfirst.storage.GUIDBinding;

/**
 * Simple wrapper for keeping cache entry metadata.
 * 
 * @author Robert Moore
 *
 */
public class CacheEntry {
  /**
   * The source of the cache entry.
   */
  private final transient CacheOrigin origin;
  
  /**
   * The bindings for the entry.
   */
  private final transient Collection<GUIDBinding> bindings;
  
  /**
   * Creates a new cache entry for the collection of bindings.
   * @param bindings the set of bindings to cache.
   * @param origin the origin of the bindings (type of message).
   */
  public CacheEntry(final Collection<GUIDBinding> bindings, final CacheOrigin origin){
    super();
    this.origin = origin;
    this.bindings = bindings;
  }
  
  public CacheEntry(final CacheOrigin origin, final GUIDBinding... bindings){
    super();
    this.origin = origin;
    LinkedList<GUIDBinding> bindList = new LinkedList<GUIDBinding>();
    for(int i = 0; i < bindings.length; ++i){
      bindList.add(bindings[i]);
    }
    this.bindings = bindList;
  }
  
  /**
   * Gets the origin of this cache entry.
   * @return the origin of this cache entry.
   */
  public CacheOrigin getOrigin(){
    return this.origin;
  }
  
  /**
   * Gets the bindings for this cache entry.
   * @return
   */
  public Collection<GUIDBinding> getBindings(){
    return this.bindings;
  }
}
